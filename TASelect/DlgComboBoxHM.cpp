#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "TASelectDoc.h"

#include "Global.h"
#include "Utilities.h"
#include "Hydronic.h"
#include "HMInclude.h"
#include "DlgComboBoxHM.h"
#include "Select.h"
#include "SelectPM.h"
#include "ProductSelectionParameters.h"

IMPLEMENT_DYNAMIC(CDlgComboBoxHM,CDlgSpreadCB)

CDlgComboBoxHM::CDlgComboBoxHM( CSSheet *pOwnerSSheet, CWaterChar* pWC )
	: CDlgSpreadCB( IDD, pOwnerSSheet )
{
	m_eSCB = eSCB::SCB_Last;

	// Initialize member variables.
	m_pRankList = new CRank( false );

	if( NULL != pWC ) 
	{
		m_pWC = pWC;
	}

	m_pCB = NULL;
}

CDlgComboBoxHM::~CDlgComboBoxHM( void )
{
	if( NULL != m_pRankList )
	{
		delete m_pRankList;
		m_pRankList = NULL;
	}

	if( NULL != m_pCB )
	{
		delete m_pCB;
		m_pCB = NULL;
	}
}

void CDlgComboBoxHM::OpenDialogSCB( CDS_HydroMod *pHM, eSCB eSCBtype, long lColumn, long lRow )
{
	m_eSCB = eSCBtype;
	m_pHM = pHM;
	SetMainSheetPos( m_pMainSheet, lColumn, lRow );

	CString str = m_pMainSheet->GetCellText( lColumn, lRow );
	
	switch( m_eSCB )
	{
		case SCB_CircuitPrimaryPipeSerie:
		case SCB_DistributionSupplyPipeSerie:
		case SCB_DistributionReturnPipeSerie:
		case SCB_CircuitSecondaryPipeSerie:
			_ShowCBPipeSerie();
			break;
		
		case SCB_CircuitPrimaryPipeSize:
		case SCB_DistributionSupplyPipeSize:
		case SCB_DistributionReturnPipeSize:
		case SCB_CircuitSecondaryPipeSize:
			_ShowCBPipeSize();
			break;
		
		case SCB_BvLoc:		//Primary Bv valve used as measuring valve
			_ShowCBBvLoc( m_pHM );
			break;

		case SCB_BvPName:	//Primary
			_ShowCBBvName( m_pHM->GetpBv() );
			break;

		case SCB_BvPType:	//Primary
			_ShowCBBvType( m_pHM->GetpBv() );
			break;
		
		case SCB_BvPConnection:	//Primary
			_ShowCBBvConnect( m_pHM->GetpBv() );
			break;
		
		case SCB_BvPVersion:	//Primary
			_ShowCBBvVersion( m_pHM->GetpBv() );
			break;
		
		case SCB_BvPPN:	//Primary
			_ShowCBBvPN( m_pHM->GetpBv() );
			break;
		
		case SCB_BvBName:	//Bypass
			_ShowCBBvName( m_pHM->GetpBypBv() );
			break;

		case SCB_BvBType:	//Bypass
			_ShowCBBvType( m_pHM->GetpBypBv() );
			break;

		case SCB_BvBConnection:	//Bypass
			_ShowCBBvConnect( m_pHM->GetpBypBv() );
			break;

		case SCB_BvBVersion:	//Bypass
			_ShowCBBvVersion( m_pHM->GetpBypBv() );
			break;

		case SCB_BvBPN:	//Bypass
			_ShowCBBvPN( m_pHM->GetpBypBv() );
			break;
		
		case SCB_BvSName:	//Secondary
			_ShowCBBvName( m_pHM->GetpSecBv() );
			break;

		case SCB_BvSType:	//Secondary
			_ShowCBBvType( m_pHM->GetpSecBv() );
			break;

		case SCB_BvSConnection:	//Secondary
			_ShowCBBvConnect( m_pHM->GetpSecBv() );
			break;

		case SCB_BvSVersion:	//Secondary
			_ShowCBBvVersion( m_pHM->GetpSecBv() );
			break;

		case SCB_BvSPN:	//Secondary
			_ShowCBBvPN( m_pHM->GetpSecBv() );
			break;

		case SCB_DpC:
			_ShowCBDpC();
			break;
		
		case SCB_DpCType:
			_ShowCBDpCType();
			break;
		
		case SCB_DpCConnection:
			_ShowCBDpCConnect();
			break;
		
		case SCB_DpCPN:
			_ShowCBDpCPN();
			break;

		case SCB_CvLoc:
			_ShowCBCvLoc(m_pHM);
			break;

		case SCB_Cv:
			_ShowCBCv();
			break;
		
		case SCB_CvKvs:
			_ShowCBCvKvs();
			break;
		
		case SCB_CvConnection:
			_ShowCBCvConnect();
			break;
		
		case SCB_CvVersion:
			_ShowCBCvVersion();
			break;
		
		case SCB_CvPN:
			_ShowCBCvPN();
			break;
		
		case SCB_ShutOffValve:
			_ShowCBShutOffValve();
			break;

		case SCB_ShutOffValveType:
			_ShowCBShutOffValveType();
			break;

		case SCB_ShutOffValveConnection:
			_ShowCBShutOffValveConnect();
			break;

		case SCB_ShutOffValveVersion:
			_ShowCBShutOffValveVersion();
			break;

		case SCB_ShutOffValvePN:
			_ShowCBShutOffValvePN();
			break;
		
		case SCB_DpCBCValve:
			_ShowCBDpCBCValve();
			break;

		case SCB_DpCBCValveConnection:
			_ShowCBDpCBCValveConnect();
			break;

		case SCB_DpCBCValveVersion:
			_ShowCBDpCBCValveVersion();
			break;

		case SCB_DpCBCValvePN:
			_ShowCBDpCBCValvePN();
			break;

		case SCB_SmartControlValve:
			_ShowCBSmartControlValve();
			break;

		case SCB_SmartControlValveBody:
			_ShowCBSmartControlValveBodyMaterial();
			break;

		case SCB_SmartControlValveConnection:
			_ShowCBSmartControlValveConnect();
			break;

		case SCB_SmartControlValvePN:
			_ShowCBSmartControlValvePN();
			break;

		case SCB_SmartDpC:
			_ShowCBSmartDpC();
			break;

		case SCB_SmartDpCBody:
			_ShowCBSmartDpCBodyMaterial();
			break;

		case SCB_SmartDpCConnection:
			_ShowCBSmartDpCConnect();
			break;

		case SCB_SmartDpCPN:
			_ShowCBSmartDpCPN();
			break;

		// Hub direct selection, list comes from directly from DB
		case SCB_DS_HubValveName:
		case SCB_DS_HubDpCName:
		case SCB_DS_StationValveType:
		case SCB_DS_StationValveCtrlType:
		case SCB_DS_StationValveName:
		case SCB_DS_StationActuatorType:
		case SCB_DS_ShutOffValve:
			InitList( m_pHM, m_eSCB );
			_ShowList(  lColumn, lRow );
			break;

		case SCB_TU_Flow:
			_ShowCBTUFlow();
			break;

		case SCB_TU_Dp:
			_ShowCBTUDp();
			break;
	}
	
	if( NULL == m_pCB->GetSafeHwnd() )
	{
		return;
	}
	
	if( 1 == m_pCB->GetMaxCols() )
	{
		// 'GetClientRect' returns the rectangle coordinates of the client area
		CRect rectClient;
		m_pCB->GetClientRect( &rectClient );

		// Convert number of columns to spread column width
		double dColWidth;
		m_pCB->LogUnitsToColWidth( rectClient.Width(), &dColWidth );

		// 'GetMaxTextColWidthW' returns the width of the widest text string in the specified column.
		double dMaxColWidth = m_pCB->GetMaxTextColWidthW( 1 );

		if( dMaxColWidth > dColWidth )
		{
			m_pCB->SetColWidth( 1, dMaxColWidth );
		}
		else
		{
			m_pCB->SetColWidth( 1, dColWidth );
		}
	}
}

LPARAM CDlgComboBoxHM::GetLParamSelected( void )
{
	LPARAM lParam = NULL;
	CString str;

	if( NULL != m_pCB )
	{
		m_pCB->GetRow( 1, m_pCB->GetSelectedRow(), str, lParam );
	}

	return lParam;
}

void CDlgComboBoxHM::InitList( CDS_HydroMod *pHM, eSCB eSCBtype )
{
	m_eSCB = eSCBtype;
	m_pHM = pHM;
	
	switch( m_eSCB )
	{
		case SCB_DS_HubValveName:
			_InitListHubValveName();
			break;

		case SCB_DS_HubDpCName:
			_InitListHubDpCName();
			break;

		case SCB_DS_StationValveType:
			_InitListStationValveType();
			break;
	
		case SCB_DS_StationValveCtrlType:
			_InitListStationValveCtrlType();
			break;

		case SCB_DS_StationValveName:
			_InitListStationValveName();
			break;
	
		case SCB_DS_StationActuatorType:
			_InitListStationActuatorType();
			break;

		case SCB_DS_ShutOffValve:
			_InitListShutOffValve();
			break;
	}
}

bool CDlgComboBoxHM::GetFirstListItem( CString &str, LPARAM &itemdata ) 
{
	if( FALSE == m_pRankList->GetFirst( str, itemdata ) )
	{
		str = _T("");
		itemdata = (LPARAM)0;
		return false;
	}

	return true;
}

void CDlgComboBoxHM::Close()
{
	BeginWaitCursor();
	_CloseDialogSCB();
	OnCloseCB( 0, 0 );
}

BEGIN_MESSAGE_MAP( CDlgComboBoxHM, CDlgSpreadCB )
	ON_WM_CREATE()
	ON_WM_PAINT()
	// Spread DLL messages.
	ON_MESSAGE( SSM_CLICK , ClickFpspread )
	ON_MESSAGE( SSM_DBLCLK , DblClickFpspread )
	ON_MESSAGE( SSM_LEAVECELL, LeaveCellFpspread )
END_MESSAGE_MAP()

void CDlgComboBoxHM::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

BOOL CDlgComboBoxHM::OnInitDialog()
{
	CDlgSpreadCB::OnInitDialog();
	CRect rect( 0, 0, 50, 50 );
	
	// To avoid memory leaks, delete CSpreadComboBox if already exist (previous usage)
	if( NULL != m_pCB )
	{
		delete m_pCB;
	}

	m_pCB = new CSpreadComboBox();
	ASSERT( NULL != m_pCB );

	if( NULL != m_pCB )
	{
		// GetStyle() returns style with WS_POPUP set! In Spread OCX we can call 'Create' with this style (because spreadsheet calls CWnd::CreateControl)
		// but not with the DLL version (spreadsheet calls CWnd:Create that not allow WS_CHILD and WS_POPUP at the same time).
		if( !m_pCB->Create( ( ( GetStyle() & ~(WS_POPUP) ) | WS_CHILD | WS_BORDER ) ^ WS_VISIBLE, rect, this, IDC_FPSPREAD ) )
		{
			return false;
		}
	}

	m_pCB->Init();
	m_pCB->SetBool( SSB_ALLOWDRAGDROP, FALSE );

	Refresh();
	
	return TRUE;
}

BOOL CDlgComboBoxHM::PreTranslateMessage(MSG* pMsg)
{
	BOOL BReturn = CDialogExt::PreTranslateMessage(pMsg);

	// Notify Enter key to the parent window
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		long lColumn, lRow;
		m_pCB->GetActiveCell( &lColumn, &lRow );
		m_pCB->CellClicked( lColumn, lRow );
	}
	// Notify Escape key pressed
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		//Do nothing;
	}

	return BReturn;
}

void CDlgComboBoxHM::Refresh()
{
	// Compute size needed for combo box spread sheet
	GetDlgItem( IDC_FPSPREAD )->ShowWindow( SW_SHOW );
	
	// Initialize SpreadComboBox Owner
	m_pCB->SetOwner( this );

	// Combo Sheet Size
	CRect rect;

	if( m_pCB->GetLastVisibleCol() > m_pCB->GetMaxCols() || m_pCB->GetLastVisibleRow() > m_pCB->GetMaxRows() )
	{
		rect = m_pCB->GetSheetSizeInPixels();
	}
	else
	{
		rect = m_pCB->GetSelectionInPixels( 1, 1, m_pCB->GetLastVisibleCol(), m_pCB->GetLastVisibleRow() );
	}
	
	// Take in count scroll bar width
	if( m_pCB->GetMaxRows() > m_pCB->GetVisibleRows() )
	{
		rect.right += 20;
	}

	// Cell position in MainSheet
	CRect Srect = GetComboPos( rect );
	
	// Combo minimum width is main cell width 
	int CellWidth = Srect.Width();
	int rectwidth = rect.Width();

	if( rectwidth < CellWidth )
	{
		rect.right = rect.left + CellWidth;
	}
	
	::SetWindowPos( this->m_hWnd, HWND_TOPMOST, Srect.left, Srect.top, rect.Width(), rect.Height() + (int)m_pCB->GetFontSize( 0, 0 ), SWP_SHOWWINDOW );
	m_pCB->Invalidate();
	m_pCB->SetFocus();
}

void CDlgComboBoxHM::OnPaint()
{
	CPaintDC dc( this );
	CRect rect;
	GetClientRect( (LPRECT)rect );

	if( NULL != m_pCB->GetSafeHwnd() )
	{
		m_pCB->MoveWindow( rect.left, rect.top, rect.Width(), rect.Height(), TRUE );
	}
}

int CDlgComboBoxHM::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( CDlgSpreadCB::OnCreate( lpCreateStruct ) == -1 )
	{
		return -1;
	}
	
	return 0;
}

LRESULT CDlgComboBoxHM::ClickFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;

	if( NULL != m_pCB )
	{
		m_pCB->CellClicked( pCellCoord->Col, pCellCoord->Row );
	}

	return 0;
}

LRESULT CDlgComboBoxHM::DblClickFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;

	if( NULL != m_pCB )
	{
		m_pCB->DblClick( pCellCoord->Col, pCellCoord->Row );
	}

	return 0;
}

LRESULT CDlgComboBoxHM::LeaveCellFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_LEAVECELL *pLeaveCell = (SS_LEAVECELL *)lParam;
	BOOL fCancel = FALSE;

	if( NULL != m_pCB )
	{
		m_pCB->LeaveCell( pLeaveCell->ColCurrent, pLeaveCell->RowCurrent, pLeaveCell->ColNew, pLeaveCell->RowNew, &fCancel );
	}

	return fCancel;
}

void CDlgComboBoxHM::_CloseDialogSCB()
{
	BeginWaitCursor();
	
	CString str;
	LPARAM lParam = 0;
	m_bModified = false;
	CDS_HydroMod *pHM = m_pHM;
	
	m_pCB->GetRow( 1, m_pCB->GetSelectedRow(), str, lParam );
	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	// test on lParam removed due to possibility to get MvLoc in primary == 0
	//
	if( true == pPrjParam->IsFreezed() )
	{
		EndWaitCursor();	
		return;									// no selection has been done
	}
	
	switch( GetSCBType() )
	{
		case CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSize:
		case CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSize:
		case CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSize:
		case CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSize:
			{
				CPipes *pPipe = (CPipes *)GetLParam();
				ASSERT( NULL != pPipe );

				if( lParam == (LPARAM)( pPipe->GetIDPtr().MP ) )
				{
					break;	// nothing change
				}
				
				if( NULL != lParam )
				{
					pPipe->SetLock( !pPipe->IsBestPipe( (CData *)lParam ) );
					pPipe->SetIDPtr( ( (CDB_Pipe*)lParam )->GetIDPtr() );

					// HYS-1735: Update secondary pipe for 3-way mixing circuit.
					if( CDB_CircSchemeCateg::e3wTypeMixing == pHM->GetpSchcat()->Get3WType() )
					{
						pHM->GetpCircuitSecondaryPipe()->SetLock( !pHM->GetpCircuitSecondaryPipe()->IsBestPipe( (CData*)lParam ) );
						pHM->GetpCircuitSecondaryPipe()->SetIDPtr( ( (CDB_Pipe*)lParam )->GetIDPtr() );
					}
				}
				else
				{
					pPipe->SetIDPtr( _NULL_IDPTR );
				}

				m_bModified = true;
			}
			break;

		case CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSerie:
		case CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSerie:
		case CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSerie:
		case CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSerie:
			{
				CPipes *pPipe = (CPipes*)GetLParam();
				ASSERT( NULL != pPipe );

				if( NULL == pPipe )
				{
					break;
				}
				
				if( NULL == lParam || lParam == (LPARAM)pPipe->GetPipeSeries() )
				{
					break;	// nothing change
				}
				
				if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == pPipe->GetLocate() )
				{
					pPrjParam->GetpHmCalcParams()->SetPrjParamID( CPrjParams::_PrjParamsID::PipeCircSerieID, ( (CTable *)lParam )->GetIDPtr().ID );
					pHM->GetpCircuitPrimaryPipe()->SetPipeSeries( (CTable *)lParam );
					
					// HYS-1735: Update secondary pipe for 3-way mixing circuit.
					if( CDB_CircSchemeCateg::e3wTypeMixing == pHM->GetpSchcat()->Get3WType() )
					{
						pHM->GetpCircuitSecondaryPipe()->SetPipeSeries( (CTable*)lParam );
					}
				}
				else if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == pPipe->GetLocate() )
				{
					pPrjParam->GetpHmCalcParams()->SetPrjParamID( CPrjParams::_PrjParamsID::PipeDistSupplySerieID, ( (CTable *)lParam )->GetIDPtr().ID );
					pHM->GetpDistrSupplyPipe()->SetPipeSeries( (CTable *)lParam );
				}
				else if( CDS_HydroMod::eHMObj::eDistributionReturnPipe == pPipe->GetLocate() )
				{
					pPrjParam->GetpHmCalcParams()->SetPrjParamID( CPrjParams::_PrjParamsID::PipeDistReturnSerieID, ( (CTable *)lParam )->GetIDPtr().ID );
					pHM->GetpDistrReturnPipe()->SetPipeSeries( (CTable *)lParam );
				}
				else if( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe == pPipe->GetLocate() )
				{
					//pPrjParam->GetpHmCalcParams()->SetPrjParamID( CPrjParams::_PrjParamsID::PipeDistSe, ( (CTable *)lParam )->GetIDPtr().ID );
					pHM->GetpCircuitSecondaryPipe()->SetPipeSeries( (CTable *)lParam );
				}
				else
				{
					break;
				}

				pPipe->SetLock( false );
				m_bModified = true;
			}
			break;
		
		case eSCB::SCB_BvLoc:		//Primary Bv valve used as measuring valve
			{
				CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV *)GetLParam();
				ASSERT( NULL != pBv );

				if( NULL == pBv )
				{
					break;
				}

				if( NULL == pHM->GetpDpC() )
				{
					break;
				}

				if( (LPARAM)( pHM->GetpDpC()->GetMvLoc() ) == lParam )
				{
					break;	// nothing change
				}

				pHM->GetpDpC()->SetMvLoc((eMvLoc)lParam);
				pHM->GetpDpC()->SetMvLocLocked(true);
				m_bModified = true;
			}
			break;

		case eSCB::SCB_BvBName:
		case eSCB::SCB_BvPName:
		case eSCB::SCB_BvSName:
			{
				CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV *)GetLParam();
				ASSERT( NULL != pBv );

				if( NULL == lParam || NULL == pBv )
				{
					break;
				}
				
				if( (LPARAM)( pBv->GetIDPtr().MP ) == lParam )
				{
					break;	// nothing change
				}
				
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *) lParam );

				if (NULL != pTAP)
				{
					// Automatically locked when differ from best choice done in ForceBVSelection
					pBv->ForceBVSelection( pTAP->GetIDPtr() );
				}

				m_bModified = true;
			}
			break;

		case eSCB::SCB_BvPType:	//Primary
		case eSCB::SCB_BvBType:	//Bypass
		case eSCB::SCB_BvSType:	//Secondary
			{
				CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV *)GetLParam();
				ASSERT( NULL != pBv );

				if( NULL == lParam || NULL == pBv )
				{
					break;
				}
				
				if( pBv->GetpSelBVType() == ( (CDB_StringID *)lParam ) )
				{
					break;	// nothing change
				}
				
				// New user selection unlock BV
				m_pHM->SetLock( pBv->GetHMObjectType(), false, false );
				pBv->SetpSelBVTypeAccordingUserChoice( (CDB_StringID *)lParam );
			
				m_bModified = true;
			}
			break;

		case eSCB::SCB_BvPConnection:	//Primary
		case eSCB::SCB_BvBConnection:	//Bypass
		case eSCB::SCB_BvSConnection:	//Secondary
			{
				CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV *)GetLParam();
				ASSERT( NULL != pBv );

				if( NULL == lParam || NULL == pBv )
				{
					break;
				}
				
				if( pBv->GetpSelBVConn() == ( (CDB_StringID *)lParam ) )
				{
					break;	// nothing change
				}
				
				// New user selection unlock BV
				m_pHM->SetLock( pBv->GetHMObjectType(), false, false );
				pBv->SetpSelBVConnAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_BvPVersion:	//Primary
		case eSCB::SCB_BvBVersion:	//Bypass
		case eSCB::SCB_BvSVersion:	//Secondary
			{
				CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV *)GetLParam();
				ASSERT( NULL != pBv );

				if( NULL == lParam || NULL == pBv )
				{
					break;
				}
				
				if( pBv->GetpSelBVVers() == ( (CDB_StringID *)lParam ) )
				{
					break;	// nothing change
				}
				
				// New user selection unlock BV
				m_pHM->SetLock( pBv->GetHMObjectType(), false, false );
				pBv->SetpSelBVVersAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_BvPPN:	//Primary
		case eSCB::SCB_BvBPN:	//Bypass
		case eSCB::SCB_BvSPN:	//Secondary
			{
				CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV *)GetLParam();
				ASSERT( NULL != pBv );

				if( NULL == lParam || NULL == pBv )
				{
					break;
				}
				
				if( pBv->GetpSelBVConn() == ( (CDB_StringID *)lParam ) )
				{
					break;	// nothing change
				}
				
				// New user selection unlock BV 
				m_pHM->SetLock( pBv->GetHMObjectType(), false, false );
				pBv->SetpSelBVPNAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_DpC:
			{
				CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
				ASSERT( NULL != pDpC );

				if( NULL == lParam || NULL == pDpC )
				{
					break;
				}
			
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *) lParam );

				if (NULL != pTAP)
				{
					pDpC->ForceDpCSelection ( pTAP->GetIDPtr() );
					m_bModified = true;
				}
			}
			break;

		case eSCB::SCB_DpCType:
			{
				CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
				ASSERT( NULL != pDpC );

				if( NULL == lParam || NULL == pDpC )
				{
					break;
				}
				
				if( (LPARAM)pDpC->GetpSelDpCType() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock DpC
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eDpC, false, false );
				pDpC->SetpSelDpCTypeAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_DpCConnection:
			{
				CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
				ASSERT( NULL != pDpC );

				if( NULL == lParam || NULL == pDpC )
				{
					break;
				}
				
				if( (LPARAM)pDpC->GetpSelDpCConn() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock BV
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eDpC, false, false );
				pDpC->SetpSelDpCConnAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_DpCPN:
			{
				CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
				ASSERT( NULL != pDpC );

				if ( NULL == lParam || NULL == pDpC )
				{
					break;
				}
				
				if( (LPARAM)pDpC->GetpSelDpCPN() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock BV
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eDpC, false, false );
				pDpC->SetpSelDpCPNAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_ShutOffValve:
			{
				if( NULL == m_pHM->GetpSch() )
				{
					break;
				}

				CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

				if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveReturn;
				}

				CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
				ASSERT( NULL != pclHMShutoffValve );

				if( NULL == lParam || NULL == pclHMShutoffValve )
				{
					break;
				}
				
				if( (LPARAM)( pclHMShutoffValve->GetIDPtr().MP ) == lParam )
				{
					break;	// nothing change
				}
				
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *) lParam );

				if (NULL != pTAP)
				{
					// Automatically locked when differ from best choice done in 'ForceShutoffValveSelection'.
					pclHMShutoffValve->ForceShutoffValveSelection( pTAP->GetIDPtr() );
				}

				m_bModified = true;
			}
			break;

		case eSCB::SCB_ShutOffValveType:
			{
				if( NULL == m_pHM->GetpSch() )
				{
					break;
				}

				CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

				if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveReturn;
				}

				CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
				ASSERT( NULL != pclHMShutoffValve );

				if( NULL == lParam || NULL == pclHMShutoffValve )
				{
					break;
				}
				
				if( (LPARAM)pclHMShutoffValve->GetpSelType() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock ShutoffValve.
				m_pHM->SetLock( eHMObj, false, false );
				pclHMShutoffValve->SetpSelTypeAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_ShutOffValveConnection:
			{
				if( NULL == m_pHM->GetpSch() )
				{
					break;
				}

				CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

				if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveReturn;
				}

				CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
				ASSERT( NULL != pclHMShutoffValve );

				if( NULL == lParam || NULL == pclHMShutoffValve )
				{
					break;
				}
				
				if( (LPARAM)pclHMShutoffValve->GetpSelConnection() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock ShutoffValve.
				m_pHM->SetLock( eHMObj, false, false );
				pclHMShutoffValve->SetpSelConnAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_ShutOffValveVersion:
			{
				if( NULL == m_pHM->GetpSch() )
				{
					break;
				}

				CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

				if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveReturn;
				}

				CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
				ASSERT( NULL != pclHMShutoffValve );

				if( NULL == lParam || NULL == pclHMShutoffValve )
				{
					break;
				}
				
				if( (LPARAM)pclHMShutoffValve->GetpSelVersion() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock ShutoffValve.
				m_pHM->SetLock( eHMObj, false, false );
				pclHMShutoffValve->SetpSelVersionAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_ShutOffValvePN:
			{
				if( NULL == m_pHM->GetpSch() )
				{
					break;
				}

				CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

				if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveReturn;
				}

				CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
				ASSERT( NULL != pclHMShutoffValve );

				if( NULL == lParam || NULL == pclHMShutoffValve )
				{
					break;
				}
				
				if( (LPARAM)pclHMShutoffValve->GetpSelPN() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock ShutoffValve.
				m_pHM->SetLock( eHMObj, false, false );
				pclHMShutoffValve->SetpSelPNAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_DpCBCValve:
			{
				CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();
				ASSERT( NULL != pclHMDpCBCValve );

				if( NULL == lParam || NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
				{
					break;
				}
				
				if( (LPARAM)( pclHMDpCBCValve->GetCvIDPtr().MP ) == lParam )
				{
					break;	// nothing change
				}
				
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *) lParam );

				if (NULL != pTAP)
				{
					// Not automatically locked when differ from best choice done in 'ForceCVSelection' !!!
					m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, !pclHMDpCBCValve->IsBestCV( (CData *)lParam ) );

					pclHMDpCBCValve->ForceCVSelection( pTAP->GetIDPtr() );

					// HYS-1868: We force CV but we need also to force actuator (Or remove if not!).
					pclHMDpCBCValve->SelectActuator();
				}

				m_bModified = true;
			}
			break;

		case eSCB::SCB_DpCBCValveConnection:
			{
				CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();
				ASSERT( NULL != pclHMDpCBCValve );

				if( NULL == lParam || NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
				{
					break;
				}
				
				if( (LPARAM)pclHMDpCBCValve->GetpSelCVConn() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock DpCBCV.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false, false );
				pclHMDpCBCValve->SetpUserChoiceCVConn( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_DpCBCValveVersion:
			{
				CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();
				ASSERT( NULL != pclHMDpCBCValve );

				if( NULL == lParam || NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
				{
					break;
				}
				
				if( (LPARAM)pclHMDpCBCValve->GetpSelCVVers() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock DpCBCV.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false, false );
				pclHMDpCBCValve->SetpUserChoiceCVVers( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_DpCBCValvePN:
			{
				CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();
				ASSERT( NULL != pclHMDpCBCValve );

				if( NULL == lParam || NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
				{
					break;
				}
				
				if( (LPARAM)pclHMDpCBCValve->GetpSelCVPN() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock DpCBCV.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false, false );
				pclHMDpCBCValve->SetpUserChoiceCVPN( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case SCB_CvLoc:
			{
				CDS_HydroMod::CCv *pHMCV = m_pHM->GetpCV();
				ASSERT( NULL != pHMCV );

				if( NULL == pHMCV )
				{
					break;
				}

				CDS_Hm3WInj *p3WInj = dynamic_cast<CDS_Hm3WInj *>( m_pHM );
				
				if( NULL != p3WInj )
				{
					CvLocation eCVLocation = (CvLocation)lParam;
					p3WInj->AdjustCircuitIDForValveLocalisation( eCVLocation );
					pHMCV->SetCvLocate( eCVLocation );
					m_bModified = true;
				}
			}
			break;

		case eSCB::SCB_Cv:
			{
				CDS_HydroMod::CCv *pCV = m_pHM->GetpCV();
				ASSERT( NULL != pCV );

				if( NULL == lParam || NULL == pCV )
				{
					break;
				}
				
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, !pCV->IsBestCV( (CData *)lParam ) );
				CData *pData = (CData *)lParam;
				pCV->ForceCVSelection( pData->GetIDPtr() );

				// HYS-1868: We force CV but we need also to force actuator (Or remove if not!).
				pCV->SelectActuator();

				m_bModified = true;
			}
			break;

		case eSCB::SCB_CvKvs:
			{
				CDS_HydroMod::CCv *pCV = m_pHM->GetpCV();
				ASSERT( NULL != pCV );

				if( NULL == lParam || NULL == pCV )
				{
					break;
				}
				
				m_pCB->GetRow( 1, m_pCB->GetSelectedRow(), str, lParam );
				double kvs = CDimValue::CUtoSI( _C_KVCVCOEFF, lParam );	
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, !pCV->IsBestCV( kvs / 100 ) );
				pCV->SetKvs( kvs / 100 );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_CvConnection:

			if( NULL == lParam || (LPARAM)m_pHM->GetpCV()->GetpSelCVConn() == lParam )
			{
				break;	// nothing change
			}
				
			// New user selection unlock CV
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false, false );
			m_pHM->GetpCV()->SetpUserChoiceCVConn( (CDB_StringID *)lParam );
			m_bModified = true;
			break;

		case eSCB::SCB_CvVersion:

			if( NULL == lParam || (LPARAM)m_pHM->GetpCV()->GetpSelCVVers() == lParam )
			{
				break;	// nothing change.
			}
				
			// New user selection unlock CV
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false );
			m_pHM->GetpCV()->SetpUserChoiceCVVers( (CDB_StringID *)lParam );
			m_bModified = true;
			break;

		case eSCB::SCB_CvPN:

			if( NULL == lParam || (LPARAM)m_pHM->GetpCV()->GetpSelCVPN() == lParam )
			{
				break;	// nothing change
			}
				
			// New user selection unlock CV
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false );
			m_pHM->GetpCV()->SetpUserChoiceCVPN( (CDB_StringID *)lParam );
			m_bModified = true;
			break;

		case eSCB::SCB_SmartControlValve:
			{
				CDS_HydroMod::CSmartControlValve *pclHMSmartControlValve = m_pHM->GetpSmartControlValve();
				ASSERT( NULL != pclHMSmartControlValve );

				if( NULL == lParam || NULL == pclHMSmartControlValve )
				{
					break;
				}
				
				if( (LPARAM)( pclHMSmartControlValve->GetIDPtr().MP ) == lParam )
				{
					break;	// nothing change
				}
				
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *) lParam );

				if (NULL != pTAP)
				{
					// Not automatically locked when differ from best choice done in 'ForceSelection' !!!
					m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, !pclHMSmartControlValve->IsBestSmartControlValve( (CData *)lParam ) );

					pclHMSmartControlValve->ForceSelection( pTAP->GetIDPtr() );
				}

				m_bModified = true;
			}
			break;

		case eSCB::SCB_SmartControlValveBody:
			{
				CDS_HydroMod::CSmartControlValve *pclHMSmartControlValve = m_pHM->GetpSmartControlValve();
				ASSERT( NULL != pclHMSmartControlValve );

				if( NULL == lParam || NULL == pclHMSmartControlValve )
				{
					break;
				}
				
				if( (LPARAM)pclHMSmartControlValve->GetpSelBodyMaterial() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock the smart control valve.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, false, false );
				pclHMSmartControlValve->SetpSelBodyMaterialAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_SmartControlValveConnection:
			{
				CDS_HydroMod::CSmartControlValve *pclHMSmartControlValve = m_pHM->GetpSmartControlValve();
				ASSERT( NULL != pclHMSmartControlValve );

				if( NULL == lParam || NULL == pclHMSmartControlValve )
				{
					break;
				}
				
				if( (LPARAM)pclHMSmartControlValve->GetpSelConnection() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock the smart control valve.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, false, false );
				pclHMSmartControlValve->SetpSelConnectionAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_SmartControlValvePN:
			{
				CDS_HydroMod::CSmartControlValve *pclHMSmartControlValve = m_pHM->GetpSmartControlValve();
				ASSERT( NULL != pclHMSmartControlValve );

				if( NULL == lParam || NULL == pclHMSmartControlValve )
				{
					break;
				}
				
				if( (LPARAM)pclHMSmartControlValve->GetpSelPN() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock the smart control valve.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, false, false );
				pclHMSmartControlValve->SetpSelPNAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_SmartDpC:
			{
				CDS_HydroMod::CSmartDpC *pclHMSmartDpC = m_pHM->GetpSmartDpC();
				ASSERT( NULL != pclHMSmartDpC );

				if( NULL == lParam || NULL == pclHMSmartDpC )
				{
					break;
				}
				
				if( (LPARAM)( pclHMSmartDpC->GetIDPtr().MP ) == lParam )
				{
					break;	// nothing change
				}
				
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *) lParam );

				if (NULL != pTAP)
				{
					// Not automatically locked when differ from best choice done in 'ForceSelection' !!!
					m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, !pclHMSmartDpC->IsBestValve( (CData *)lParam ) );

					pclHMSmartDpC->ForceSelection( pTAP->GetIDPtr() );
				}

				m_bModified = true;
			}
			break;

		case eSCB::SCB_SmartDpCBody:
			{
				CDS_HydroMod::CSmartDpC *pclHMSmartDpC = m_pHM->GetpSmartDpC();
				ASSERT( NULL != pclHMSmartDpC );

				if( NULL == lParam || NULL == pclHMSmartDpC )
				{
					break;
				}
				
				if( (LPARAM)pclHMSmartDpC->GetpSelBodyMaterial() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock the smart differential pressure controller.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, false, false );
				pclHMSmartDpC->SetpSelBodyMaterialAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_SmartDpCConnection:
			{
				CDS_HydroMod::CSmartDpC *pclHMSmartDpC = m_pHM->GetpSmartDpC();
				ASSERT( NULL != pclHMSmartDpC );

				if( NULL == lParam || NULL == pclHMSmartDpC )
				{
					break;
				}
				
				if( (LPARAM)pclHMSmartDpC->GetpSelConnection() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock the smart differential pressure controller.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, false, false );
				pclHMSmartDpC->SetpSelConnectionAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		case eSCB::SCB_SmartDpCPN:
			{
				CDS_HydroMod::CSmartDpC *pclHMSmartDpC = m_pHM->GetpSmartDpC();
				ASSERT( NULL != pclHMSmartDpC );

				if( NULL == lParam || NULL == pclHMSmartDpC )
				{
					break;
				}
				
				if( (LPARAM)pclHMSmartDpC->GetpSelPN() == lParam )
				{
					break;	// nothing change
				}
				
				// New user selection unlock the smart differential pressure controller.
				m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, false, false );
				pclHMSmartDpC->SetpSelPNAccordingUserChoice( (CDB_StringID *)lParam );
				m_bModified = true;
			}
			break;

		// Hub Direct Selection
		case SCB_DS_HubValveName:

			if ( NULL == lParam )
			{
				break;
			}

			( (CDS_HmHub *)m_pHM )->SetReturnValveID( ( (CDB_HubValv *)lParam )->GetIDPtr().ID );
			( (CDS_HmHub *)m_pHM )->SetLock( CDS_HydroMod::eHMObj::eBVprim, true, false );
			( (CDS_HmHub *)m_pHM )->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
			break;

		case SCB_DS_HubDpCName:

			if ( NULL == lParam )
			{
				break;
			}

			( (CDS_HmHub *)m_pHM )->SetReturnValveID( ( (CDB_HubValv *)lParam )->GetIDPtr().ID );
			( (CDS_HmHub *)m_pHM )->SetLock( CDS_HydroMod::eHMObj::eDpC, true, false );
			( (CDS_HmHub *)m_pHM )->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
			break;

		case SCB_DS_ShutOffValve:

			if ( NULL == lParam )
			{
				break;
			}

			( (CDS_HmHub *)m_pHM )->SetReturnValveID( ( (CDB_HubValv *)lParam )->GetIDPtr().ID );
			( (CDS_HmHub *)m_pHM )->SetLock( CDS_HydroMod::eHMObj::eShutoffValveReturn, true, false );
			( (CDS_HmHub *)m_pHM )->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
			break;

		case SCB_DS_StationValveType:

			if ( NULL == lParam )
			{
				break;
			}

			( (CDS_HmHubStation *)m_pHM )->Init( ( (CDB_StringID *)lParam )->GetIDPtr().ID, (CTable *)m_pHM->GetParent(), ( (CDS_HmHubStation *)m_pHM )->GetCvTaKv() );
			( (CDS_HmHubStation *)m_pHM )->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
			break;

		case SCB_DS_StationValveCtrlType:

			if ( NULL == lParam )
			{
				break;
			}

			if( ( (CDS_HmHubStation *)m_pHM )->GetpCV()->GetCtrlType() != (CDB_ControlProperties::CvCtrlType)( lParam - 1 ) )
			{
				// Force reset of actuator
				( (CDS_HmHubStation *)m_pHM )->SetReturnActuatorID( _T("") );
				( (CDS_HmHubStation *)m_pHM )->SetReturnValveID( _T("") );
				( (CDS_HmHubStation *)m_pHM )->GetpCV()->SetCtrlType( (CDB_ControlProperties::CvCtrlType)( lParam - 1 ) );
				( (CDS_HmHubStation *)m_pHM )->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
			}

			break;

		case SCB_DS_StationValveName:

			if ( NULL == lParam )
			{
				break;
			}

			( (CDS_HmHubStation *)m_pHM )->SetReturnValveID( ( (CDB_HubStaValv *)lParam )->GetIDPtr().ID );
			( (CDS_HmHubStation *)m_pHM )->SetLock( CDS_HydroMod::eHMObj::eBVprim, true, false );
			( (CDS_HmHubStation *)m_pHM )->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
			
			m_bModified = true;
			break;

		case SCB_DS_StationActuatorType:

			if( NULL != lParam && ( ( (CDS_HmHubStation *)m_pHM )->GetBalTypeID() == _T("RVTYPE_BVC") ) )
			{
				if( -1 == lParam )
				{
					( (CDS_HmHubStation *)m_pHM )->SetReturnActuatorID( _T("") );
				}
				else
				{
					( (CDS_HmHubStation *)m_pHM )->SetReturnActuatorID( ( (CDB_Actuator *)lParam )->GetIDPtr().ID );
				}
			}

			break;

		// Terminal unit combo boxes.
		case SCB_TU_Flow:
			{
				lParam--;
				CTermUnit::_QType eQType = (CTermUnit::_QType)( GetLParam() - 1 );
				// If nothing change..

				if( lParam < 0 || eQType == (CTermUnit::_QType)lParam )
				{
					break;
				}

				// HYS-1882: I don't know why we don't call SetQ ot or SetPDT when we change QMode, but it is necessary to call it to reset old power when 
				// we switch from PDT (with DT different than the regime) to the flow mode.  
				m_pHM->GetpTermUnit()->SetQType( (CTermUnit::_QType)lParam );
				if( CTermUnit::_QType::Q == m_pHM->GetpTermUnit()->GetQType() )
				{
					m_pHM->GetpTermUnit()->SetQ( m_pHM->GetpTermUnit()->GetQ(), false );
				}

				m_bModified = true;
			}
			break;

		case SCB_TU_Dp:
			{
				CDS_HydroMod::eDpType DpType = (CDS_HydroMod::eDpType)GetLParam();

				// If nothing change..
				if( NULL == lParam || DpType == ( CDS_HydroMod::eDpType )lParam )
				{
					break;
				}

				switch( lParam )
				{
					case CDS_HydroMod::eDpType::Dp:
						// Change 'eDpType' and recompute Dp in regards to current available variables.
						m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, m_pHM->GetpTermUnit()->GetDp() );
						break;

					case CDS_HydroMod::eDpType::QDpref:
						{
							// Change 'eDpType'.
							double dQ = m_pHM->GetpTermUnit()->GetQ();
							double dDp = m_pHM->GetpTermUnit()->GetDp();
							double dDpRef = m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDpRef;
							double dQRef = m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dQRef;
							
							if( 0.0 == dQRef && 0.0 == dDpRef )
							{
								m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, dDp, dQ );
							}
							else if( dQRef > 0.0 && dDpRef > 0.0 )
							{
								m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, DBL_MAX, DBL_MAX );
							}
							else
							{
								if( dQ > 0.0 && dDp > 0.0 )
								{
									// HYS-1716: To compute the power of the terminal unit, we need the temperature that is on the supply.
									CWaterChar *pclWaterChar = m_pHM->GetpOutWaterChar( CAnchorPt::PipeLocation_Supply );
									ASSERT( NULL != pclWaterChar );

									if( NULL != pclWaterChar )
									{
										if( dQRef > 0.0  )
										{
											double dKv = CalcKv( dQ, dDp, pclWaterChar->GetDens() );
											dDpRef = CalcDp( dQRef, dKv, pclWaterChar->GetDens() );
											m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, dDpRef, DBL_MAX );
											m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDpRef = dDpRef;
										}
										else if( dDpRef > 0.0 )
										{
											double dKv = CalcKv( dQ, dDp, pclWaterChar->GetDens() );
											dQRef = CalcqT( dKv, dDpRef, pclWaterChar->GetDens() );
											m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, DBL_MAX, dQRef );
											m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dQRef = dQRef;
										}
									}
								}
							}
						}
						break;

					case CDS_HydroMod::eDpType::Kv:
						// Change 'eDpType' and Kv value.
						m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Kv, m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dKv );
						break;

					case CDS_HydroMod::eDpType::Cv:
						// Change 'eDpType' and Cv value.
						m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Cv, m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dCv );
						break;
				}
	
				m_bModified = true;
			}
			break;
	};
	
	EndWaitCursor();
}

void CDlgComboBoxHM::_ShowCBPipeSerie()
{
	CTable *pPipeSerie = NULL;
	CPipes *pPipe = NULL;
	
	// Circuit Pipe or distribution pipe
	if( eSCB::SCB_CircuitPrimaryPipeSerie == m_eSCB )
	{
		pPipe = m_pHM->GetpCircuitPrimaryPipe();
	}
	else if( eSCB::SCB_DistributionSupplyPipeSerie == m_eSCB )
	{
		pPipe = m_pHM->GetpDistrSupplyPipe();
	}
	else if( eSCB::SCB_DistributionReturnPipeSerie == m_eSCB )
	{
		pPipe = m_pHM->GetpDistrReturnPipe();
	}
	else if( eSCB::SCB_CircuitSecondaryPipeSerie == m_eSCB )
	{
		pPipe = m_pHM->GetpCircuitSecondaryPipe();
	}

	ASSERT( NULL != pPipe );

	if( NULL == pPipe )
	{
		return;
	}
	
	pPipeSerie = pPipe->GetPipeSeries();
	SetLParam( (LPARAM)pPipe );
		
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	CRank PipeList;
	m_pTADB->GetPipeSerieList( &PipeList, CTADatabase::FilterSelection::ForHMCalc );
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( FALSE );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	CString str;
	LPARAM lParam = 0;

	for( BOOL fContinue = PipeList.GetFirst( str, lParam ); TRUE == fContinue; fContinue = PipeList.GetNext( str, lParam ) )
	{
		m_pCB->SetRowParameters( 1, -1, str, lParam );
	}

	if( NULL != pPipeSerie )
	{
		m_pCB->FindAndSelectRow( 1, pPipeSerie->GetName() );
	}
	else
	{
		// PipeSerie doesn't exist yet, take it from PrjParams.
		CString strPipeID;
		CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
		ASSERT( NULL != pPrjParam );

		if( eSCB::SCB_CircuitPrimaryPipeSerie == m_eSCB )
		{
			strPipeID = pPrjParam->GetpHmCalcParams()->GetPrjParamID( CPrjParams::PipeCircSerieID );
		}
		else if( eSCB::SCB_DistributionSupplyPipeSerie == m_eSCB )
		{
			strPipeID = pPrjParam->GetpHmCalcParams()->GetPrjParamID( CPrjParams::PipeDistSupplySerieID );
		}
		else if( eSCB::SCB_DistributionReturnPipeSerie == m_eSCB )
		{
			strPipeID = pPrjParam->GetpHmCalcParams()->GetPrjParamID( CPrjParams::PipeDistReturnSerieID );
		}
		else
		{
			ASSERT( 0 );
		}
		
		if( false == strPipeID.IsEmpty() )
		{
			CTable* pTab = TASApp.GetpPipeDB()->GetPipeTab(0, false);
			ASSERT( NULL != pTab );
			pPipeSerie = (CTable *)( pTab->Get( (LPCTSTR)strPipeID ).MP );
			m_pCB->FindAndSelectRow( 1, pPipeSerie->GetName() );
		}
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
	PipeList.PurgeAll();
}

void CDlgComboBoxHM::_ShowCBPipeSize()
{
	// Define shown pipe size above and below the best pipe
	const int iPipeSizeAbove = 6;
	const int iPipeSizeBelow = 4;
	CTable *pPipeSerie = NULL;
	CPipes *pclHMPipe = NULL;
	
	// Circuit Pipe or distribution pipe
	if( eSCB::SCB_CircuitPrimaryPipeSize == m_eSCB )
	{
		pclHMPipe = m_pHM->GetpCircuitPrimaryPipe();
	}
	else if( eSCB::SCB_DistributionSupplyPipeSize == m_eSCB )
	{
		pclHMPipe = m_pHM->GetpDistrSupplyPipe();
	}
	else if( eSCB::SCB_DistributionReturnPipeSize == m_eSCB )
	{
		pclHMPipe = m_pHM->GetpDistrReturnPipe();
	}
	else if( eSCB::SCB_CircuitSecondaryPipeSize == m_eSCB )
	{
		pclHMPipe = m_pHM->GetpCircuitSecondaryPipe();
	}
	
	if( NULL == pclHMPipe )
	{
		ASSERT_RETURN;
	}

	SetLParam( (LPARAM)pclHMPipe );
	pPipeSerie = pclHMPipe->GetPipeSeries();

	if( NULL == pPipeSerie )
	{
		ASSERT_RETURN;
	}
	
	// A flow must exist to select a pipe.
	if( 0.0 != pclHMPipe->GetRealQ() )
	{
		if( 0 == GetSafeHwnd() )
		{
			Create();
		}
		else
		{
			Refresh();
		}

		CProductSelelectionParameters clProductSelectionParameters( TASApp.GetpTADB(), TASApp.GetpTADS(), TASApp.GetpUserDB(),
				TASApp.GetpPipeDB(), m_pWC, pPipeSerie->GetIDPtr().ID );

		CSelectPipeList clSelectPipeList;
		clSelectPipeList.SelectPipes( &clProductSelectionParameters, pclHMPipe->GetRealQ(), false, false, iPipeSizeAbove, iPipeSizeBelow );
		CDB_Pipe *pclBestPipe = clSelectPipeList.GetBestPipe();
		
		// Fill spread ComboBox.
		m_pCB->Empty();
		m_pCB->SetRedraw( false );
		m_pCB->SetnCols( 3, 3 );
		m_pCB->SetnRows( 7 );
		
		// Set columns title.
		CString str; 
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_PIPESIZE );
		m_pCB->SetRowParameters( 1, -1, str, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_PIPELINDP );
		m_pCB->SetRowParameters( 2, 1, str, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_PIPEVELOCITY );
		m_pCB->SetRowParameters( 3, 1, str, NULL );
		
		m_pCB->SetCellBorder( 1, 1, true, SS_BORDERTYPE_RIGHT );
		m_pCB->SetCellBorder( 2, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		m_pCB->SetCellBorder( 3, 1, true, SS_BORDERTYPE_LEFT );

		// Units.
		m_pCB->SetRowParameters( 1, -1, _T(""), NULL );

		TCHAR tcUnitName[_MAXCHARS];
		GetNameOf( m_pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), tcUnitName );
		m_pCB->SetRowParameters( 2, 2, tcUnitName, NULL );

		GetNameOf( m_pUnitDB->GetDefaultUnit( _U_VELOCITY ), tcUnitName );
		m_pCB->SetRowParameters( 3, 2, tcUnitName, NULL );
		
		m_pCB->SetCellBorder( 1, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );
		m_pCB->SetCellBorder( 2, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		m_pCB->SetCellBorder( 3, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );

		// Freeze row title.
		m_pCB->SetFreeze( 0 , 2 );
		
		// Fill rows.
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
		int iSelectedPipe = 0;
		int iHighLightRow = 0;

		int i = 0;
		CSelectPipe *pclSelectPipe = clSelectPipeList.GetFirstSelectPipe();

		while( NULL != pclSelectPipe )
		{
			CDB_Pipe *pclPipe = pclSelectPipe->GetpPipe();

			if( NULL == pclPipe )
			{
				ASSERT_RETURN;
			}

			// Save Pipe IDPTR.MP.
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			long lRow = m_pCB->SetRowParameters( 1, -1, pclPipe->GetName(), (LPARAM)( pclPipe->GetIDPtr().MP ) );
			
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
			m_pCB->SetRowParameters( 2, lRow, _T(""), NULL );
			m_pCB->SetCUDouble( 2, lRow, _U_LINPRESSDROP, pclSelectPipe->GetLinDp() );

			m_pCB->SetRowParameters( 3, lRow, _T(""), NULL );
			m_pCB->SetCUDouble( 3, lRow, _U_VELOCITY, pclSelectPipe->GetU() );

			m_pCB->SetCellBorder( 1, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
			m_pCB->SetCellBorder( 2, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
			m_pCB->SetCellBorder( 3, lRow, true, SS_BORDERTYPE_LEFT );

			// A pipe is already selected use it in drop down.
			if( _T('\0') != *pclHMPipe->GetIDPtr().ID )
			{
				if( pclHMPipe->GetIDPtr().MP == pclPipe->GetIDPtr().MP )
				{
					iSelectedPipe = i + 3;	// +3 2 header rows and i start at 0
				}
			}
			else
			{
				// Select best pipe row.
				if( NULL != pclBestPipe && pclBestPipe->GetIDPtr().MP == pclPipe->GetIDPtr().MP )
				{
					iSelectedPipe = i + 3;	// +3 2 header rows and i start at 0
				}
			}
			
			// HighLigth best pipe.
			if( NULL != pclBestPipe && pclBestPipe->GetIDPtr().MP == pclPipe->GetIDPtr().MP )
			{
				m_pCB->SetBackColor( 0, i + 3, m_pCB->GetMaxCols(), i + 3, _SOFTLIGHTGREEN );
			}

			pclSelectPipe = clSelectPipeList.GetNextSelectPipe();
			i++;
		}

		if( 0 == iSelectedPipe )
		{	
			// Current selected pipe is not present in the list it will be added.
			m_pCB->SetMaxRows( m_pCB->GetMaxRows() + 1 );
			m_pCB->InsRow( 3 );
			m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			
			long lRow = m_pCB->SetRowParameters( 1, -1, ( (CDB_Pipe *)( pclHMPipe->GetIDPtr().MP ) )->GetName(), (LPARAM)( pclHMPipe->GetIDPtr().MP ) );
			iSelectedPipe = lRow;

			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
			m_pCB->SetRowParameters( 2, lRow, _T(""), NULL );
			m_pCB->SetCUDouble( 2, lRow, _U_LINPRESSDROP, pclHMPipe->GetLinDp() );

			m_pCB->SetRowParameters( 3, lRow, _T(""), NULL );
			m_pCB->SetCUDouble( 3, lRow, _U_VELOCITY, pclHMPipe->GetVelocity() );

			m_pCB->SetCellBorder( 1, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
			m_pCB->SetCellBorder( 2, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
			m_pCB->SetCellBorder( 3, lRow, true, SS_BORDERTYPE_LEFT );
		}

		m_pCB->SelectRow( iSelectedPipe );
	}
}

void CDlgComboBoxHM::_ShowCBBvLoc( CDS_HydroMod *pHMBV )
{
	if( NULL == pHMBV || NULL == pHMBV->GetpDpC() )
	{
		return;
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	// Fill spread ComboBox.
	SetLParam( (LPARAM)pHMBV->GetpBv() );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );

	int iSelRow = -1;

	// Test if primary is possible.
	if( -1.0 != pHMBV->GetpDpC()->GetDplmin() && pHMBV->GetpDpC()->GetDplmin() < pHMBV->GetpDpC()->GetDpToStab( eMvLoc::MvLocPrimary ) )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGCBHM_MVONPRIMARY );
		LPARAM lParam = (LPARAM)( eMvLoc::MvLocPrimary );
		int j = m_pCB->SetRowParameters( 1, -1, str, lParam );

		if( eMvLoc::MvLocPrimary == pHMBV->GetpDpC()->GetMvLoc() )
		{
			iSelRow = j;
		}
	}

	CString str = TASApp.LoadLocalizedString( IDS_DLGCBHM_MVONSECONDARY );
	LPARAM lParam = (LPARAM)( eMvLoc::MvLocSecondary );
	int j = m_pCB->SetRowParameters( 1, -1, str, lParam );

	if( eMvLoc::MvLocSecondary == pHMBV->GetpDpC()->GetMvLoc() )
	{
		iSelRow = j;
	}

	m_pCB->SelectRow( iSelRow );
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBCvLoc( CDS_HydroMod *pHM )
{
	if( NULL == pHM )
	{
		return;
	}
	
	CDS_Hm3WInj *pHM3WInj = dynamic_cast<CDS_Hm3WInj *>(pHM);
	
	if( NULL == pHM3WInj )
	{
		return;
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	// Fill spread ComboBox.
	SetLParam( (LPARAM)pHM );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );

	int iSelRow = -1;

	CString str = TASApp.LoadLocalizedString( IDS_DLGCBHM_CVONAUTO );
	LPARAM lParam = (LPARAM)(0);	// Auto
	int j = m_pCB->SetRowParameters( 1, -1, str, lParam );
	
	if( true == pHM->GetpCV()->IsCVLocAuto() ) 
	{
		iSelRow = j;
	}
	
	str = TASApp.LoadLocalizedString( IDS_DLGCBHM_CVONPRIMARY );
	lParam = (LPARAM)( CvLocation::CvLocPrimSide );
	j = m_pCB->SetRowParameters( 1, -1, str, lParam );
	
	if( eb3True == pHM->GetpCV()->IsCVLocInPrimary() )
	{
		if ( -1 == iSelRow )
		{
			iSelRow = j;
		}
	}
	str = TASApp.LoadLocalizedString( IDS_DLGCBHM_CVONSECONDARY );
	lParam = (LPARAM)( CvLocation::CvLocSecSide );
	j = m_pCB->SetRowParameters( 1, -1, str, lParam );
	
	if( eb3False == pHM->GetpCV()->IsCVLocInPrimary() )
	{
		if ( -1 == iSelRow )
		{
			iSelRow = j;
		}
	}

	m_pCB->SelectRow( iSelRow );
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBBvName( CDS_HydroMod::CBV *pHMBV )
{
	ASSERT( NULL != pHMBV );

	if( NULL == pHMBV )
	{
		return;
	}
	
	std::multimap <int, CValvesData > *pMainValveMap = pHMBV->GetpMapValves();
	std::multimap <int, CValvesData >::iterator ItMainValveMap;
	
	// When MainValveMap is empty force filling list with all possible BV
	if( 0 == pMainValveMap->size() ) 
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		m_pHM->ComputeAll();
	}
	
	// No possible choice
	if( 0 == pMainValveMap->size() ) 
	{
		return;
	}

	// Variables
	long lBvName		= 1;
	long lBvSize		= 2;
	long lBvConnection	= 3;
	long lBvVersion		= 4;
	long lBvSetting		= 5;
	long lBvDp			= 6;
	long lBvDpFullOp	= 7;
	long lBvPN			= 8;
	long lLast			= 9;	
	bool bDisplayConnect = false;
	bool bDisplayVersion = false;
	bool bDisplayPN = false;

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	// Fill spread ComboBox.
	SetLParam( (LPARAM)pHMBV );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Set columns title.
	int iMaxCols = lLast - 1;
	m_pCB->SetnCols( iMaxCols, iMaxCols );
	m_pCB->SetnRows( lLast -1 );
	
	CString str; 
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVNAME );
	m_pCB->SetRowParameters( lBvName, -1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVSIZE );
	m_pCB->SetRowParameters( lBvSize, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVPCONNECT );
	m_pCB->SetRowParameters( lBvConnection, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVPVERSION );
	m_pCB->SetRowParameters( lBvVersion, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVPSETTING );
	m_pCB->SetRowParameters( lBvSetting, 1, str, NULL );
	
	ItMainValveMap = pMainValveMap->begin();

	if( false == ( (CDB_TAProduct *)( ItMainValveMap->second.GetpTAP() ) )->IsKvSignalEquipped() )
	{
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVPDP );
		m_pCB->SetRowParameters( lBvDp, 1, str, NULL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVPDPSIGNAL );
		m_pCB->SetRowParameters( lBvDp, 1, str, NULL );
	}	

	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVPDPFULLOPEN );
	m_pCB->SetRowParameters( lBvDpFullOp, 1, str, NULL );

	str = TASApp.LoadLocalizedString( IDS_PN );
	m_pCB->SetRowParameters( lBvPN, 1, str, NULL );

	// Border.
	m_pCB->SetCellBorder( lBvName, 1, true, SS_BORDERTYPE_RIGHT );
	
	for( int i = lBvSize; i < iMaxCols; i++ )
	{
		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( iMaxCols, 1, true, SS_BORDERTYPE_LEFT );

	// Units.
	TCHAR unitname[_MAXCHARS];
	m_pCB->SetRowParameters( lBvName, -1, L"", NULL );
	m_pCB->SetRowParameters( lBvSize, 2, L"", NULL );
	m_pCB->SetRowParameters( lBvConnection, 2, L"", NULL );
	m_pCB->SetRowParameters( lBvVersion, 2, L"", NULL );
	str = TASApp.LoadLocalizedString( IDS_TURNS );
	m_pCB->SetRowParameters( lBvSetting, 2, str, NULL );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	m_pCB->SetRowParameters( lBvDp, 2, unitname, NULL );
	m_pCB->SetRowParameters( lBvDpFullOp, 2, unitname, NULL );
	m_pCB->SetRowParameters( lBvPN, 2, L"", NULL );
	
	// Border.
	m_pCB->SetCellBorder( lBvName, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );
	
	for( int i = lBvSize; i < iMaxCols; i++ )
	{
		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( iMaxCols, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
	
	// Freeze row title.
	m_pCB->SetFreeze( 0, 2 );
	
	// Fill rows.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	long lSelectedRow = 0;
	long lHighLightRow = 0;

	for( ItMainValveMap = pMainValveMap->begin(); pMainValveMap->end() != ItMainValveMap; ++ItMainValveMap )
	{
		CDB_TAProduct *pTAP = ItMainValveMap->second.GetpTAP();

		if( NULL == pTAP )
		{
			continue;
		}

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		// HYS-1221 : To show deleted products in red color in products list Bv.
		if( true == pTAP->IsDeleted() )
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		long lRow = m_pCB->SetRowParameters( lBvName, -1, pTAP->GetName(), (LPARAM)( pTAP ) );
		
		if( true == pHMBV->IsBestBv( pTAP ) )
		{
			lHighLightRow = lRow;
		}
		
		if( pHMBV->GetpTAP() == pTAP )
		{
			lSelectedRow = lRow;
		}

		// Display the "Connection" column if not all Bv have the same one
		if( NULL != pHMBV->GetpTAP()  && pHMBV->GetpTAP()->GetConnectIDPtr().MP != pTAP->GetConnectIDPtr().MP )
		{
			bDisplayConnect = true;
		}
		
		// Display the "Version" column if not all Bv have the same one
		if( NULL != pHMBV->GetpTAP()  && pHMBV->GetpTAP()->GetVersionIDPtr().MP != pTAP->GetVersionIDPtr().MP )
		{
			bDisplayVersion = true;
		}

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lBvSize, lRow, ( (CDB_StringID *)( pTAP->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lBvConnection, lRow, ( (CDB_StringID *)( pTAP->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lBvVersion, lRow, ( (CDB_StringID *)( pTAP->GetVersionIDPtr().MP ) )->GetString(), NULL );

		CDB_ValveCharacteristic *pChar = pTAP->GetValveCharacteristic();
		str = pChar->GetSettingString( ItMainValveMap->second.GetSetting() );

		m_pCB->SetRowParameters( lBvSetting, lRow, str, NULL );
		m_pCB->SetRowParameters( lBvDp, lRow, WriteCUDouble( _U_DIFFPRESS, ItMainValveMap->second.GetDp() ), NULL );
		
		if( false == pTAP->IsKvSignalEquipped() )
		{
			m_pCB->SetRowParameters( lBvDpFullOp, lRow, WriteCUDouble( _U_DIFFPRESS, ItMainValveMap->second.GetDpFO() ), NULL );
		}
		else
		{
			// DpSignal exist CS, VV DpFO is stored into Dp3
			m_pCB->SetRowParameters( lBvDpFullOp, lRow, WriteCUDouble( _U_DIFFPRESS, ItMainValveMap->second.GetSignal() ), NULL );
		}

		m_pCB->SetRowParameters( lBvPN, lRow, ( (CDB_StringID *)( pTAP->GetPNIDPtr().MP ) )->GetString(), NULL );
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
		// Border.
		m_pCB->SetCellBorder( lBvName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i = lBvSize; i < iMaxCols; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
		
		m_pCB->SetCellBorder( iMaxCols, lRow, true, SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetBackColor(0,lHighLightRow,m_pCB->GetMaxCols(),lHighLightRow,_SOFTLIGHTGREEN);
	
	// Current selected BV isn't in the current list. Add it at the first position.
	if( 0 == lSelectedRow && NULL != dynamic_cast<CDB_RegulatingValve *>( pHMBV->GetpTAP() ) )
	{
		CDB_TAProduct *pclRegulatingValve = (CDB_RegulatingValve *)( pHMBV->GetpTAP() );
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		long lRow = m_pCB->SetRowParameters( lBvName, -1, pclRegulatingValve->GetName(), (LPARAM)pclRegulatingValve );
		lSelectedRow = lRow;
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lBvSize, lRow, ( (CDB_StringID *)pclRegulatingValve->GetSizeIDPtr().MP )->GetString(), NULL );
		m_pCB->SetRowParameters( lBvConnection, lRow, ( (CDB_StringID *)pclRegulatingValve->GetConnectIDPtr().MP )->GetString(), NULL );
		m_pCB->SetRowParameters( lBvVersion, lRow, ( (CDB_StringID *)pclRegulatingValve->GetVersionIDPtr().MP)->GetString(), NULL );

		CDB_Characteristic *pclCharacteristic = pclRegulatingValve->GetValveCharacteristic();
		str = pclCharacteristic->GetSettingString( pHMBV->GetSetting() );
		m_pCB->SetRowParameters( lBvSetting, lRow, str, NULL );
		
		if( false == pHMBV->IsKvSignalEquipped() )
		{
			m_pCB->SetRowParameters( lBvDp, lRow, WriteCUDouble( _U_DIFFPRESS, pHMBV->GetDp() ), NULL );
		}
		else
		{
			m_pCB->SetRowParameters( lBvDp, lRow, WriteCUDouble( _U_DIFFPRESS, pHMBV->GetDpSignal() ), NULL );
		}

		m_pCB->SetRowParameters( lBvDpFullOp, lRow, WriteCUDouble( _U_DIFFPRESS, pHMBV->GetDpMin() ), NULL );
		m_pCB->SetRowParameters( lBvPN, lRow, ( (CDB_StringID *)pclRegulatingValve->GetPNIDPtr().MP)->GetString(), NULL );

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
		// Border.
		m_pCB->SetCellBorder( lBvName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i = lBvSize; i < iMaxCols; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
		
		m_pCB->SetCellBorder( iMaxCols, lRow, true, SS_BORDERTYPE_LEFT );
	}

	m_pCB->SelectRow( lSelectedRow );

	// Hide unnecessary columns.
	if( false == bDisplayConnect )
	{
		_HideUnnecessaryCol( lBvConnection, lBvConnection );
	}
	
	if( false == bDisplayVersion )
	{
		_HideUnnecessaryCol( lBvVersion, lBvVersion );
	}

	if( false == bDisplayPN )
	{
		_HideUnnecessaryCol( lBvPN, lBvPN );
	}
	
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBBvType( CDS_HydroMod::CBV *pHMBV )
{
	ASSERT( NULL != pHMBV );

	if( NULL == pHMBV )
	{
		return;
	}
	
	CRankEx SortList;
	
	// Get current selected pipe.
	CDB_Pipe *pPipe = (CDB_Pipe *)( m_pHM->GetpPipe( pHMBV->GetHMObjectType() )->GetIDPtr().MP );

	if( NULL == pPipe )
	{
		return; // No pipe selected
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Find DN field based on Pipe size and SizeBelow and Size Above from technical parameters
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );
	int iDN15 = m_pclTableDN->GetSize( _T("DN_15") );
	int iDNMax = __max( iDN + m_pTechParam->GetSizeShiftAbove(), iDN15 );
	int iDNMin = __max( iDN + m_pTechParam->GetSizeShiftBelow(), 0 );

	// Verify if the valve is used for a DpC.
	bool bUsedAsMV = false;

	if( CDS_HydroMod::eHMObj::eBVprim == pHMBV->GetHMObjectType() )
	{
		if( NULL != pHMBV->GetpParentHM()->GetpSch() && CDB_CircuitScheme::eDpCTypeNU != pHMBV->GetpParentHM()->GetpSch()->GetDpCType() )
		{
			bUsedAsMV = true;
			bool fRemoveSTAM = ( eMvLoc::MvLocSecondary == pHMBV->GetpParentHM()->GetpDpC()->GetMvLoc() );
			m_pTADB->GetBVTypeList( &SortList, CTADatabase::FilterSelection::ForHMCalc, iDNMin, iDNMax, true, fRemoveSTAM, true );
		}
	}
	
	if( false == bUsedAsMV ) 
	{
		m_pTADB->GetBVTypeList( &SortList, CTADatabase::FilterSelection::ForHMCalc, iDNMin, iDNMax, true, true, false );
	}

	// Fill spread ComboBox.
	SetLParam( (LPARAM)pHMBV );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	LPARAM lParam;
	_string str;

	for( bool fContinue = SortList.GetFirst( str, lParam ); true == fContinue; fContinue = SortList.GetNext( str, lParam ) )
	{
		m_pCB->SetRowParameters( 1, -1, ( (CDB_StringID *)lParam )->GetString(), lParam );
	}
	
	if( NULL != pHMBV->GetpSelBVType() )
	{
		m_pCB->FindAndSelectRow( 1, pHMBV->GetpSelBVType()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBBvConnect( CDS_HydroMod::CBV *pHMBV )
{
	ASSERT( NULL != pHMBV );

	if( NULL == pHMBV )
	{
		return;
	}
	
	// Fill the connection combo according to selected type.
	if( NULL == pHMBV->GetpSelBVType() ) 
	{
		pHMBV->SetpSelBVConnAccordingUserChoice( NULL );
		return;
	}

	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pHMBV->GetAvailableConnections( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Fill spread ComboBox.
	SetLParam( (LPARAM)pHMBV );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );

	// Start filling row.
	for ( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}

	if( NULL != pHMBV->GetpSelBVConn() )
	{
		m_pCB->FindAndSelectRow( 1, pHMBV->GetpSelBVConn()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBBvVersion( CDS_HydroMod::CBV *pHMBV )
{
	ASSERT( NULL != pHMBV );

	if( NULL == pHMBV )
	{
		return;
	}
	
	// Fill the connection combo according to selected type.
	if( NULL == pHMBV->GetpSelBVType() || NULL == pHMBV->GetpSelBVConn() )
	{
		pHMBV->SetpSelBVVersAccordingUserChoice( NULL );
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pHMBV->GetAvailableVersions( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Fill spread ComboBox.
	SetLParam( (LPARAM)pHMBV );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	// Start filling row.
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}

	if( NULL != pHMBV->GetpSelBVVers() )
	{
		m_pCB->FindAndSelectRow( 1, pHMBV->GetpSelBVVers()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBBvPN( CDS_HydroMod::CBV *pHMBV )
{
	ASSERT( NULL != pHMBV );

	if( NULL == pHMBV )
	{
		return;
	}
	
	// Fill the connection combo according to selected type.
	if( NULL == pHMBV->GetpSelBVType() || NULL == pHMBV->GetpSelBVConn() || NULL == pHMBV->GetpSelBVVers() ) 
	{
		pHMBV->SetpSelBVPNAccordingUserChoice( NULL );
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pHMBV->GetAvailablePN( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	// Fill spread ComboBox.
	SetLParam( (LPARAM)pHMBV );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	// Start filling row.
	for ( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != pHMBV->GetpSelBVPN() )
	{
		m_pCB->FindAndSelectRow( 1, pHMBV->GetpSelBVPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpC()
{
	CDS_HydroMod::CDpC *pHMDpC = m_pHM->GetpDpC();

	if( NULL == pHMDpC )
	{
		return;
	}

	// Variables
	long lDpCName			= 1;
	long lDpCSize			= 2;
	long lDpCType			= 3;
	long lDpCConnection		= 4;
	long lDpCVersion		= 5;
	long lDpCDpLRange		= 6;
	long lDpCSetting		= 7;
	long lDpCDpmin			= 8;
	long lDpCPN				= 9;
	long lLastDpC			= 10;
	bool bDisplayType	 = false;
	bool bDisplayConnect = false;
	bool bDisplayVersion = false;
	bool bDisplayPN	= false;

	// Force filling list with all possible DpC.
	if( NULL == pHMDpC->GetpSelDpCType() || NULL == pHMDpC->GetpSelDpCConn() || NULL == pHMDpC->GetpSelDpCPN() ) 
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		m_pHM->ComputeAll();
	}
	
	if( NULL == pHMDpC->GetpSelDpCType() || NULL == pHMDpC->GetpSelDpCConn() || NULL == pHMDpC->GetpSelDpCPN() )
	{
		return;
	}
	
	std::multimap <int, CDpCData > *pMapDpC = pHMDpC->GetpMapDpC();
	std::multimap <int, CDpCData >::iterator ItMapDpC = pMapDpC->end();
	
	if( 0 == pMapDpC->size() )
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceDpC );
		m_pHM->ComputeAll();
	}

	// No possible choice
	if( 0 == pMapDpC->size() ) 
	{
		return;
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( lLastDpC - 1, lLastDpC - 1 );
	m_pCB->SetnRows( 7 );

	// Set columns title.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
	
	CString str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCNAME );
	m_pCB->SetRowParameters( lDpCName, -1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCSIZE );
	m_pCB->SetRowParameters( lDpCSize, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCTYPE );
	m_pCB->SetRowParameters( lDpCType, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCCONNECT );
	m_pCB->SetRowParameters( lDpCConnection, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_BVPVERSION );
	m_pCB->SetRowParameters( lDpCVersion, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCDPLRANGE );
	m_pCB->SetRowParameters( lDpCDpLRange, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCSETTING );
	m_pCB->SetRowParameters( lDpCSetting, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCDPMIN );
	m_pCB->SetRowParameters( lDpCDpmin, 1, str, NULL );

	str = TASApp.LoadLocalizedString( IDS_PN );
	m_pCB->SetRowParameters( lDpCPN, 1, str, NULL );
		
	// Border.
	m_pCB->SetCellBorder( lDpCName, 1, true, SS_BORDERTYPE_RIGHT );
	
	for( int i = lDpCSize; i < lLastDpC - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( lLastDpC - 1, 1, true, SS_BORDERTYPE_LEFT );
	
	// Units.
	TCHAR unitname[_MAXCHARS];
	m_pCB->SetRowParameters( lDpCName, -1, L"", NULL );
	m_pCB->SetRowParameters( lDpCSize, 2, L"", NULL );
	m_pCB->SetRowParameters( lDpCType, 2, L"", NULL );
	m_pCB->SetRowParameters( lDpCConnection, 2, L"", NULL );
	m_pCB->SetRowParameters( lDpCVersion, 2, L"", NULL );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	m_pCB->SetRowParameters( lDpCDpLRange, 2, unitname, NULL );
	str = TASApp.LoadLocalizedString( IDS_TURNS );
	m_pCB->SetRowParameters( lDpCSetting, 2, str, NULL );
	m_pCB->SetRowParameters( lDpCDpmin, 2, unitname, NULL );
	m_pCB->SetRowParameters( lDpCPN, 2, L"", NULL );
	
	// Border.
	m_pCB->SetCellBorder( lDpCName, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );
	
	for( int i = lDpCSize; i < lLastDpC - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( lLastDpC - 1, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
	
	// Freeze row title.
	m_pCB->SetFreeze( 0 , 2 );
	
	// Fill rows.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	long lSelectedRow = 0;
	long lHighLightRow = 0;
	
	for( ItMapDpC = pMapDpC->begin(); pMapDpC->end() != ItMapDpC; ++ItMapDpC )
	{
		CDB_DpController *pDpC = ItMapDpC->second.GetpDpC();

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

		// HYS-1221 : To show deleted products in red color in products list DpC.
		if( true == pDpC->IsDeleted() )
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		long lRow = m_pCB->SetRowParameters( lDpCName, -1, pDpC->GetName(), (LPARAM) pDpC );

		if( true == pHMDpC->IsBestDpC( pDpC) )
		{
			lHighLightRow = lRow;
		}
		
		if( pHMDpC->GetIDPtr().MP == pDpC )
		{
			lSelectedRow = lRow;
		}

		// Display the "Type" column if not all Dpc have the same one.
		if( NULL != pHMDpC->GetIDPtr().MP && 0 != IDcmp( ( (CDB_DpController *)( pHMDpC->GetIDPtr().MP ) )->GetTypeID(), pDpC->GetTypeID() ) )
		{
			bDisplayType = true;
		}

		// Display the "Connection" column if not all Dpc have the same one.
		if( NULL != pHMDpC->GetIDPtr().MP && 0 != IDcmp( ( (CDB_DpController *)( pHMDpC->GetIDPtr().MP ) )->GetConnectID(), pDpC->GetConnectID() ) )
		{
			bDisplayConnect = true;
		}

		// Display the "Version" column if not all Dpc have the same one.
		if( NULL != pHMDpC->GetIDPtr().MP && 0 != IDcmp( ( (CDB_DpController *)( pHMDpC->GetIDPtr().MP ) )->GetVersionID(), pDpC->GetVersionID() ) )
		{
			bDisplayVersion = true;
		}

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lDpCSize, lRow, ( (CDB_StringID *)(pDpC->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCType, lRow, ( (CDB_StringID *)(pDpC->GetTypeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCConnection, lRow, ( (CDB_StringID *)(pDpC->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCVersion, lRow, ( (CDB_StringID *)(pDpC->GetVersionIDPtr().MP ) )->GetString(), NULL );
				
		// Dpl range.
		str = pDpC->GetFormatedDplRange( false ).c_str();
		m_pCB->SetRowParameters( lDpCDpLRange, lRow, str, NULL );
		
		// Change color if Dp to stabilize is out of range.
		if( eb3False == pHMDpC->CheckDpLRange( pDpC ) )
		{
			m_pCB->SetForeColor( lDpCDpLRange, lRow, _RED );
		}
		else
		{
			m_pCB->SetForeColor( lDpCDpLRange, lRow, _BLACK );
		}
		
		double dSetting = pHMDpC->GetDpCSetting( pDpC );

		str = _T("-");
		CDB_TAProduct *pTAP = pHMDpC->GetpTAP();

		if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
		{
			str = pTAP->GetValveCharacteristic()->GetSettingString( dSetting );
		}

		m_pCB->SetRowParameters( lDpCSetting, lRow, str, NULL );
		m_pCB->SetRowParameters( lDpCDpmin, lRow, WriteCUDouble( _U_DIFFPRESS, ItMapDpC->second.GetDpMin() ), NULL );

		m_pCB->SetRowParameters( lDpCPN, lRow, ( (CDB_StringID *)(pDpC->GetPNIDPtr().MP ) )->GetString(), NULL );
						
		// Border.
		m_pCB->SetCellBorder( lDpCName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i =lDpCSize; i < lLastDpC - 1; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
		
		m_pCB->SetCellBorder( lLastDpC - 1, lRow, true, SS_BORDERTYPE_LEFT );
	
	}
	
	m_pCB->SetBackColor(0,lHighLightRow,m_pCB->GetMaxCols(),lHighLightRow,_SOFTLIGHTGREEN);

	// Current existing DpC doesn't exist into the list, add it.
	if( 0 == lSelectedRow && pHMDpC->GetIDPtr().MP )
	{
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
		CDB_DpController *pDpC = (CDB_DpController *)( pHMDpC->GetIDPtr().MP );
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
		long lRow = m_pCB->SetRowParameters( lDpCName, -1, pDpC->GetName(), (LPARAM)pDpC );
		
		lSelectedRow = lRow;
		
		if( pHMDpC->GetIDPtr().MP == pDpC )
		{
			lSelectedRow = lRow;
		}
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lDpCSize, lRow, ( (CDB_StringID *)( pDpC->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCType, lRow, ( (CDB_StringID *)( pDpC->GetTypeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCConnection, lRow, ( (CDB_StringID *)( pDpC->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCVersion, lRow, ( (CDB_StringID *)( pDpC->GetVersionIDPtr().MP ) )->GetString(), NULL );
				
		// Dpl range.
		str = pDpC->GetFormatedDplRange( false ).c_str();
		m_pCB->SetRowParameters( lDpCDpLRange, lRow, str, NULL );
		
		m_pCB->SetRowParameters( lDpCDpmin, lRow, WriteCUDouble( _U_DIFFPRESS, pHMDpC->GetDpmin( false ) ), NULL );
		m_pCB->SetRowParameters( lDpCPN, lRow, ( (CDB_StringID *)( pDpC->GetPNIDPtr().MP ) )->GetString(), NULL );
				
		// Border.
		m_pCB->SetCellBorder( lDpCName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i = lDpCSize; i < lLastDpC; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
	}

	m_pCB->SelectRow( lSelectedRow );
	
	// Hide unnecessary columns.
	if( false == bDisplayType )
	{
		_HideUnnecessaryCol( lDpCType, lDpCType );
	}
	
	if( false == bDisplayConnect )
	{
		_HideUnnecessaryCol( lDpCConnection, lDpCConnection );
	}

	if( false == bDisplayVersion )
	{
		_HideUnnecessaryCol( lDpCVersion, lDpCVersion );
	}

	if( false == bDisplayPN )
	{
		_HideUnnecessaryCol( lDpCPN, lDpCPN );
	}
	
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpCType()
{
	CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
	
	if( NULL == pDpC )
	{
		return;
	}
	
	m_eSCB = eSCB::SCB_DpCType;

	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pDpC->GetAvailableType( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
		
	if( NULL != m_pHM->GetpDpC()->GetpSelDpCType() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpDpC()->GetpSelDpCType()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpCConnect()
{
	// Fill the connection combo according to selected type.
	if( NULL == m_pHM->GetpDpC()->GetpSelDpCType() ) 
	{
		m_pHM->GetpDpC()->SetpSelDpCConnAccordingUserChoice( NULL );
		return;
	}

	CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
	
	if( NULL == pDpC )
	{
		return;
	}

	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pDpC->GetAvailableConnections( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Find DN field based on Pipe size and SizeBelow and Size Above from technical parameters
	m_eSCB = eSCB::SCB_DpCConnection;
	
	// Fill spread ComboBox
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}

	if( NULL !=  pDpC->GetpSelDpCConn() )
	{
		m_pCB->FindAndSelectRow( 1, pDpC->GetpSelDpCConn()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpCPN()
{
	// Fill the connection combo according to selected type.
	if( NULL == m_pHM->GetpDpC()->GetpSelDpCType() || NULL == m_pHM->GetpDpC()->GetpSelDpCConn() ) 
	{
		m_pHM->GetpDpC()->SetpSelDpCPNAccordingUserChoice( NULL );
		return;
	}

	CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();

	if( NULL == pDpC )
	{
		return;
	}

	std::map<int, CDB_StringID*> mapStrID;
	int iSize = pDpC->GetAvailablePN( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	m_eSCB = eSCB::SCB_DpCPN;
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );

	if( NULL != pDpC->GetpSelDpCPN() )
	{
		m_pCB->FindAndSelectRow( 1, pDpC->GetpSelDpCPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBShutOffValve()
{
	if( NULL == m_pHM->GetpSch() )
	{
		return;
	}

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

	if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveSupply;
	}
	else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveReturn;
	}

	CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );

	if( NULL == pclHMShutoffValve )
	{
		return;
	}

	// Force filling list with all possible shut-off valves.
	if( NULL == pclHMShutoffValve->GetpSelType() || NULL == pclHMShutoffValve->GetpSelConnection() || NULL == pclHMShutoffValve->GetpSelVersion()
			|| NULL == pclHMShutoffValve->GetpSelPN() ) 
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		m_pHM->ComputeAll();
	}
	
	if( NULL == pclHMShutoffValve->GetpSelType() || NULL == pclHMShutoffValve->GetpSelConnection() || NULL == pclHMShutoffValve->GetpSelVersion()
			|| NULL == pclHMShutoffValve->GetpSelPN() ) 
	{
		return;
	}

	std::multimap <int, CShutoffValveData > *pMapShutoffValve = pclHMShutoffValve->GetpMapShutoffValve();
	std::multimap <int, CShutoffValveData >::iterator ItMapShutoffValve = pMapShutoffValve->end();

	if( 0 == pMapShutoffValve->size() )
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceShutOffValves, (LPARAM)pclHMShutoffValve );
		m_pHM->ComputeAll();
	}

	if( 0 == pMapShutoffValve->size() )
	{
		return;
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	// Variables
	long lSVName			= 1;
	long lSVSize			= 2;
	long lSVType			= 3;
	long lSVConnection		= 4;
	long lSVVersion			= 5;
	long lSVDpFullOpening	= 6;
	long lSVPN				= 7;
	long lLastSV			= 8;
	bool bDisplayType	 = false;
	bool bDisplayConnect = false;
	bool bDisplayVersion = false;
	bool bDisplayPN = false;
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( lLastSV - 1, lLastSV - 1 );
	m_pCB->SetnRows( 7 );

	// Set columns title.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
	
	CString str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SVNAME );
	m_pCB->SetRowParameters( lSVName, -1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SVSIZE );
	m_pCB->SetRowParameters( lSVSize, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SVTYPE );
	m_pCB->SetRowParameters( lSVType, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SVCONNECT );
	m_pCB->SetRowParameters( lSVConnection, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SVVERSION );
	m_pCB->SetRowParameters( lSVVersion, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SVDPFULLOPEN );
	m_pCB->SetRowParameters( lSVDpFullOpening, 1, str, NULL );

	str = TASApp.LoadLocalizedString( IDS_PN );
	m_pCB->SetRowParameters( lSVPN, 1, str, NULL );
		
	// Border.
	m_pCB->SetCellBorder( lSVName, 1, true, SS_BORDERTYPE_RIGHT );

	for( int i = lSVSize; i < lLastSV - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}

	m_pCB->SetCellBorder( lLastSV - 1, 1, true, SS_BORDERTYPE_LEFT );
	
	// Units.
	TCHAR unitname[_MAXCHARS];
	m_pCB->SetRowParameters( lSVName, -1, L"", NULL );
	m_pCB->SetRowParameters( lSVSize, 2, L"", NULL );
	m_pCB->SetRowParameters( lSVType, 2, L"", NULL );
	m_pCB->SetRowParameters( lSVConnection, 2, L"", NULL );
	m_pCB->SetRowParameters( lSVVersion, 2, L"", NULL );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	m_pCB->SetRowParameters( lSVDpFullOpening, 2, unitname, NULL );
	m_pCB->SetRowParameters( lSVPN, 2, L"", NULL );
	
	// Border.
	m_pCB->SetCellBorder( lSVName, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );

	for( int i = lSVSize; i < lLastSV - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}

	m_pCB->SetCellBorder( lLastSV - 1, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
	
	// Freeze row title.
	m_pCB->SetFreeze( 0 , 2 );
	
	// Fill rows.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	long lSelectedRow = 0;
	long lHighLightRow = 0;
	
	for( ItMapShutoffValve = pMapShutoffValve->begin(); pMapShutoffValve->end() != ItMapShutoffValve; ++ItMapShutoffValve )
	{
		CDB_ShutoffValve *pclShutoffValve = ItMapShutoffValve->second.GetpShutoffValve();

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		long lRow = m_pCB->SetRowParameters( lSVName, -1, pclShutoffValve->GetName(), (LPARAM) pclShutoffValve );

		if( true == pclHMShutoffValve->IsBestShutoffValve( pclShutoffValve) )
		{
			lHighLightRow = lRow;
		}
		
		if( pclHMShutoffValve->GetIDPtr().MP == pclShutoffValve )
		{
			lSelectedRow = lRow;
		}

		// Display the "Type" column if not all shutoff valve have the same one.
		if( NULL != pclHMShutoffValve->GetIDPtr().MP && 0 != IDcmp( ( (CDB_ShutoffValve *)( pclHMShutoffValve->GetIDPtr().MP ) )->GetTypeID(), pclShutoffValve->GetTypeID() ) )
		{
			bDisplayType = true;
		}

		// Display the "Connection" column if not all shutoff valve have the same one.
		if( NULL != pclHMShutoffValve->GetIDPtr().MP && 0 != IDcmp( ( (CDB_ShutoffValve *)( pclHMShutoffValve->GetIDPtr().MP ) )->GetConnectID(), pclShutoffValve->GetConnectID() ) )
		{
			bDisplayConnect = true;
		}

		// Display the "Version" column if not all shutoff valve have the same one.
		if( NULL != pclHMShutoffValve->GetIDPtr().MP && 0 != IDcmp( ( (CDB_ShutoffValve *)( pclHMShutoffValve->GetIDPtr().MP ) )->GetVersionID(), pclShutoffValve->GetVersionID() ) )
		{
			bDisplayVersion = true;
		}

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lSVSize, lRow, ( (CDB_StringID *)( pclShutoffValve->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVType, lRow, ( (CDB_StringID *)( pclShutoffValve->GetTypeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVConnection, lRow, ( (CDB_StringID *)( pclShutoffValve->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVVersion, lRow, ( (CDB_StringID *)( pclShutoffValve->GetVersionIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVDpFullOpening, lRow, WriteCUDouble( _U_DIFFPRESS, ItMapShutoffValve->second.GetDp() ), NULL );
		m_pCB->SetRowParameters( lSVPN, lRow, ( (CDB_StringID *)( pclShutoffValve->GetPNIDPtr().MP ) )->GetString(), NULL );
						
		// Border.
		m_pCB->SetCellBorder( lSVName, lRow, true, SS_BORDERTYPE_RIGHT );

		for( int i = lSVSize; i < lLastSV - 1; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}

		m_pCB->SetCellBorder( lLastSV - 1, lRow, true, SS_BORDERTYPE_LEFT );
	
	}

	m_pCB->SetBackColor( 0, lHighLightRow, m_pCB->GetMaxCols(), lHighLightRow, _SOFTLIGHTGREEN );

	// Current existing shutoff valve doesn't exist into the list, add it.
	if( 0 == lSelectedRow && NULL != pclHMShutoffValve->GetIDPtr().MP )
	{
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
		CDB_ShutoffValve *pclShutoffValve = (CDB_ShutoffValve *)( pclHMShutoffValve->GetIDPtr().MP );
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
		long lRow = m_pCB->SetRowParameters( lSVName, -1, pclShutoffValve->GetName(), (LPARAM)pclShutoffValve );
		
		lSelectedRow = lRow;

		if( pclHMShutoffValve->GetIDPtr().MP == pclShutoffValve )
		{
			lSelectedRow = lRow;
		}
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lSVSize, lRow, ( (CDB_StringID *)( pclShutoffValve->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVType, lRow, ( (CDB_StringID *)( pclShutoffValve->GetTypeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVConnection, lRow, ( (CDB_StringID *)( pclShutoffValve->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVVersion, lRow, ( (CDB_StringID *)( pclShutoffValve->GetVersionIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lSVDpFullOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pclHMShutoffValve->GetDp() ), NULL );
		m_pCB->SetRowParameters( lSVPN, lRow, ( (CDB_StringID *)( pclShutoffValve->GetPNIDPtr().MP ) )->GetString(), NULL );
				
		// Border.
		m_pCB->SetCellBorder( lSVName, lRow, true, SS_BORDERTYPE_RIGHT );

		for( int i = lSVSize; i < lLastSV; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
	}

	m_pCB->SelectRow( lSelectedRow );
	
	// Hide unnecessary columns.
	if( false == bDisplayType )
	{
		_HideUnnecessaryCol( lSVType, lSVType );
	}
	
	if( false == bDisplayConnect )
	{
		_HideUnnecessaryCol( lSVConnection, lSVConnection );
	}

	if( false == bDisplayVersion )
	{
		_HideUnnecessaryCol( lSVVersion, lSVVersion );
	}

	if( false == bDisplayPN )
	{
		_HideUnnecessaryCol( lSVPN, lSVPN );
	}
	
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBShutOffValveType()
{
	if( NULL == m_pHM->GetpSch() )
	{
		return;
	}

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

	if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveSupply;
	}
	else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveReturn;
	}

	CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );

	if( NULL == pclHMShutoffValve )
	{
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pclHMShutoffValve->GetAvailableType( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_ShutOffValveType;

	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelType() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpShutoffValve( eHMObj )->GetpSelType()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBShutOffValveConnect()
{
	if( NULL == m_pHM->GetpSch() )
	{
		return;
	}

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

	if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveSupply;
	}
	else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveReturn;
	}

	CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );

	if( NULL == pclHMShutoffValve )
	{
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pclHMShutoffValve->GetAvailableConnections( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_ShutOffValveConnection;

	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelConnection() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpShutoffValve( eHMObj )->GetpSelConnection()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBShutOffValveVersion()
{
	if( NULL == m_pHM->GetpSch() )
	{
		return;
	}

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

	if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveSupply;
	}
	else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveReturn;
	}

	CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );

	if( NULL == pclHMShutoffValve )
	{
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pclHMShutoffValve->GetAvailableVersion( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_ShutOffValveVersion;

	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelVersion() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpShutoffValve( eHMObj )->GetpSelVersion()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBShutOffValvePN()
{
	if( NULL == m_pHM->GetpSch() )
	{
		return;
	}

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

	if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveSupply;
	}
	else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
	{
		eHMObj = CDS_HydroMod::eShutoffValveReturn;
	}

	CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );

	if( NULL == pclHMShutoffValve )
	{
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pclHMShutoffValve->GetAvailablePN( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_ShutOffValvePN;

	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelPN() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpShutoffValve( eHMObj )->GetpSelPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpCBCValve()
{
	CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();

	if( NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
	{
		return;
	}

	// Force filling list with all possible combined Dp controller, control and balancing valves.
	if( NULL == pclHMDpCBCValve->GetpSelCVType() || NULL == pclHMDpCBCValve->GetpSelCVConn() || NULL == pclHMDpCBCValve->GetpSelCVVers()
			|| NULL == pclHMDpCBCValve->GetpSelCVPN() ) 
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		m_pHM->ComputeAll();
	}
	
	if( NULL == pclHMDpCBCValve->GetpSelCVType() || NULL == pclHMDpCBCValve->GetpSelCVConn() || NULL == pclHMDpCBCValve->GetpSelCVVers()
			|| NULL == pclHMDpCBCValve->GetpSelCVPN() ) 
	{
		return;
	}

	std::multimap <int, CCVData > *pMapDpCBCValve = pclHMDpCBCValve->GetpMapCV();
	std::multimap <int, CCVData >::iterator ItMapDpCBCValve = pMapDpCBCValve->end();

	if( 0 == pMapDpCBCValve->size() )
	{
		return;
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	// Variables.
	long lDpCBCValveName		= 1;
	long lDpCBCValveSize		= 2;
	long lDpCBCValveType		= 3;
	long lDpCBCValveConnection	= 4;
	long lDpCBCValveVersion		= 5;
	long lDpCBCValveDpLRange	= 6;
	long lDpCBCValveSetting		= 7;
	long lDpCBCValvePN			= 8;
	long lLastDpCBCValve		= 9;

	bool bDisplayType = false;
	bool bDisplayConnect = false;
	bool bDisplayVersion = false;
	bool bDisplayPN = false;
	
	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( lLastDpCBCValve - 1, lLastDpCBCValve - 1 );
	m_pCB->SetnRows( 7 );

	// Set columns title.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
	
	CString str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCBCVNAME );
	m_pCB->SetRowParameters( lDpCBCValveName, -1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCBCVSIZE );
	m_pCB->SetRowParameters( lDpCBCValveSize, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCBCVTYPE );
	m_pCB->SetRowParameters( lDpCBCValveType, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCBCVCONNECT );
	m_pCB->SetRowParameters( lDpCBCValveConnection, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCBCVVERSION );
	m_pCB->SetRowParameters( lDpCBCValveVersion, 1, str, NULL );

	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCBCVDPLRANGE );
	m_pCB->SetRowParameters( lDpCBCValveDpLRange, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_DPCBCVPRESETTING );
	m_pCB->SetRowParameters( lDpCBCValveSetting, 1, str, NULL );

	str = TASApp.LoadLocalizedString( IDS_PN );
	m_pCB->SetRowParameters( lDpCBCValvePN, 1, str, NULL );
	
	// Border.
	m_pCB->SetCellBorder( lDpCBCValveName, 1, true, SS_BORDERTYPE_RIGHT );

	for( int i = lDpCBCValveSize; i < lLastDpCBCValve - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}

	m_pCB->SetCellBorder( lLastDpCBCValve - 1, 1, true, SS_BORDERTYPE_LEFT );
	
	// Units.
	TCHAR unitname[_MAXCHARS];
	m_pCB->SetRowParameters( lDpCBCValveName, -1, L"", NULL );
	m_pCB->SetRowParameters( lDpCBCValveSize, 2, L"", NULL );
	m_pCB->SetRowParameters( lDpCBCValveType, 2, L"", NULL );
	m_pCB->SetRowParameters( lDpCBCValveConnection, 2, L"", NULL );
	m_pCB->SetRowParameters( lDpCBCValveVersion, 2, L"", NULL );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	m_pCB->SetRowParameters( lDpCBCValveDpLRange, 2, unitname, NULL );
	str = TASApp.LoadLocalizedString( IDS_TURNS );
	m_pCB->SetRowParameters( lDpCBCValveSetting, 2, str, NULL );
	m_pCB->SetRowParameters( lDpCBCValvePN, 2, L"", NULL );
	
	// Border.
	m_pCB->SetCellBorder( lDpCBCValveName, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );

	for( int i = lDpCBCValveSize; i < lLastDpCBCValve - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}

	m_pCB->SetCellBorder( lLastDpCBCValve - 1, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
	
	// Freeze row title.
	m_pCB->SetFreeze( 0 , 2 );
	
	// Fill rows.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	long lSelectedRow = 0;
	long lHighLightRow = 0;
	
	for( ItMapDpCBCValve = pMapDpCBCValve->begin(); pMapDpCBCValve->end() != ItMapDpCBCValve; ++ItMapDpCBCValve )
	{
		CDB_DpCBCValve *pclDpCBCValve = (CDB_DpCBCValve *)ItMapDpCBCValve->second.GetpCV();

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

		// HYS-1221 : To show deleted products in red color in products list DpCBCv.
		if( true == pclDpCBCValve->IsDeleted() )
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		long lRow = m_pCB->SetRowParameters( lDpCBCValveName, -1, pclDpCBCValve->GetName(), (LPARAM) pclDpCBCValve );

		if( true == pclHMDpCBCValve->IsBestCV( pclDpCBCValve) )
		{
			lHighLightRow = lRow;
		}
		
		if( pclHMDpCBCValve->GetCvIDPtr().MP == pclDpCBCValve )
		{
			lSelectedRow = lRow;
		}

		// Display the "Type" column if not all combined Dp controller, control and balancing valve have the same one.
		if( NULL != pclHMDpCBCValve->GetCvIDPtr().MP && 0 != IDcmp( ( (CDB_DpCBCValve *)( pclHMDpCBCValve->GetCvIDPtr().MP ) )->GetTypeID(), pclDpCBCValve->GetTypeID() ) )
		{
			bDisplayType = true;
		}

		// Display the "Connection" column if not all combined Dp controller, control and balancing valve have the same one.
		if( NULL != pclHMDpCBCValve->GetCvIDPtr().MP && 0 != IDcmp( ( (CDB_DpCBCValve *)( pclHMDpCBCValve->GetCvIDPtr().MP ) )->GetConnectID(), pclDpCBCValve->GetConnectID() ) )
		{
			bDisplayConnect = true;
		}

		// Display the "Version" column if not all combined Dp controller, control and balancing valve have the same one.
		if( NULL != pclHMDpCBCValve->GetCvIDPtr().MP && 0 != IDcmp( ( (CDB_DpCBCValve *)( pclHMDpCBCValve->GetCvIDPtr().MP ) )->GetVersionID(), pclDpCBCValve->GetVersionID() ) )
		{
			bDisplayVersion = true;
		}

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lDpCBCValveSize, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCBCValveType, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetTypeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCBCValveConnection, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCBCValveVersion, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetVersionIDPtr().MP ) )->GetString(), NULL );

		CString strDplmin = ( -1.0 == ItMapDpCBCValve->second.GetDplmin() ) ? _T("x") : WriteCUDouble( _U_DIFFPRESS, ItMapDpCBCValve->second.GetDplmin() );
		CString strDplmax = ( -1.0 == ItMapDpCBCValve->second.GetDplmax() ) ? _T("x") : WriteCUDouble( _U_DIFFPRESS, ItMapDpCBCValve->second.GetDplmax() );
		str = _T("[") + strDplmin + _T("-") + strDplmax + _T("] ");
		m_pCB->SetRowParameters( lDpCBCValveDpLRange, lRow, str, NULL );

		// Change color if Dp to stabilize is out of range
		if( eb3False == pclHMDpCBCValve->CheckDpLRange( pclDpCBCValve ) )
		{
			m_pCB->SetForeColor( lDpCBCValveDpLRange, lRow, _RED );
		}
		else
		{
			m_pCB->SetForeColor( lDpCBCValveDpLRange, lRow, _BLACK );
		}

		double dSetting = ItMapDpCBCValve->second.GetSetting();

		str = _T("-");
		CDB_TAProduct *pTAP = pclHMDpCBCValve->GetpTAP();

		if( NULL != pclDpCBCValve->GetValveCharacteristic() )
		{
			str = pclDpCBCValve->GetValveCharacteristic()->GetSettingString( dSetting );
		}

		m_pCB->SetRowParameters( lDpCBCValveSetting, lRow, str, NULL );

		m_pCB->SetRowParameters( lDpCBCValvePN, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetPNIDPtr().MP ) )->GetString(), NULL );
						
		// Border.
		m_pCB->SetCellBorder( lDpCBCValveName, lRow, true, SS_BORDERTYPE_RIGHT );

		for( int i = lDpCBCValveSize; i < lLastDpCBCValve - 1; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}

		m_pCB->SetCellBorder( lLastDpCBCValve - 1, lRow, true, SS_BORDERTYPE_LEFT );
	
	}

	m_pCB->SetBackColor( 0, lHighLightRow, m_pCB->GetMaxCols(), lHighLightRow, _SOFTLIGHTGREEN );

	// Current existing combined Dp controller, control and balancing valve doesn't exist into the list, add it.
	if( 0 == lSelectedRow && NULL != pclHMDpCBCValve->GetCvIDPtr().MP )
	{
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
		CDB_DpCBCValve *pclDpCBCValve = (CDB_DpCBCValve *)( pclHMDpCBCValve->GetCvIDPtr().MP );
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
		long lRow = m_pCB->SetRowParameters( lDpCBCValveName, -1, pclDpCBCValve->GetName(), (LPARAM)pclDpCBCValve );
		
		lSelectedRow = lRow;

		if( pclHMDpCBCValve->GetCvIDPtr().MP == pclDpCBCValve )
		{
			lSelectedRow = lRow;
		}
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lDpCBCValveSize, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCBCValveType, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetTypeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCBCValveConnection, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lDpCBCValveVersion, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetVersionIDPtr().MP ) )->GetString(), NULL );
		
		CWaterChar *pclWaterChar = m_pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
		ASSERT( NULL != pclWaterChar );

		if( NULL != pclWaterChar )
		{
			std::wstring wstr = pclDpCBCValve->GetFormatedDplRange( m_pHM->GetQ(), pclWaterChar->GetDens(), false );
			m_pCB->SetRowParameters( lDpCBCValveDpLRange, lRow, CString( wstr.c_str() ), NULL );
		}

		CDB_Characteristic *pclCharacteristic = pclDpCBCValve->GetValveCharacteristic();
		str = pclCharacteristic->GetSettingString( pclHMDpCBCValve->GetSetting() );
		m_pCB->SetRowParameters( lDpCBCValveSetting, lRow, str, NULL );

		m_pCB->SetRowParameters( lDpCBCValvePN, lRow, ( (CDB_StringID *)( pclDpCBCValve->GetPNIDPtr().MP ) )->GetString(), NULL );

		// Border.
		m_pCB->SetCellBorder( lDpCBCValveName, lRow, true, SS_BORDERTYPE_RIGHT );

		for( int i = lDpCBCValveSize; i < lLastDpCBCValve; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
	}

	m_pCB->SelectRow( lSelectedRow );
	
	// Hide unnecessary columns.
	if( false == bDisplayType )
	{
		_HideUnnecessaryCol( lDpCBCValveType, lDpCBCValveType );
	}
	
	if( false == bDisplayConnect )
	{
		_HideUnnecessaryCol( lDpCBCValveConnection, lDpCBCValveConnection );
	}

	if( false == bDisplayVersion )
	{
		_HideUnnecessaryCol( lDpCBCValveVersion, lDpCBCValveVersion );
	}

	if( false == bDisplayPN )
	{
		_HideUnnecessaryCol( lDpCBCValvePN, lDpCBCValvePN );
	}
	
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpCBCValveConnect()
{
	CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();

	if( NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
	{
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pclHMDpCBCValve->GetAvailableConnections( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_DpCBCValveConnection;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpCV()->GetpSelCVConn() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpCV()->GetpSelCVConn()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpCBCValveVersion()
{
	CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();

	if( NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
	{
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pclHMDpCBCValve->GetAvailableVersions( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_DpCBCValveVersion;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpCV()->GetpSelCVPN() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpCV()->GetpSelCVPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBDpCBCValvePN()
{
	CDS_HydroMod::CCv *pclHMDpCBCValve = m_pHM->GetpCV();

	if( NULL == pclHMDpCBCValve || eb3False == pclHMDpCBCValve->IsDpCBCV() )
	{
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = pclHMDpCBCValve->GetAvailablePN( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_DpCBCValvePN;

	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpCV()->GetpSelCVPN() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpCV()->GetpSelCVPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartControlValve()
{
	CDS_HydroMod::CSmartControlValve *pHMSmartControlValve = m_pHM->GetpSmartControlValve();

	if( NULL == pHMSmartControlValve )
	{
		return;
	}

	// Variables
	long lName					= 1;
	long lSize					= 2;
	long lBodyMaterial			= 3;
	long lConnection			= 4;
	long lPN					= 5;
	long lFlowMax				= 6;
	long lPowerMax				= 7;
	long lDpMin					= 8;
	long lLast					= 9;
	bool bDisplayBodyMaterial	= false;
	bool bDisplayConnect		= false;
	bool bDisplayPN				= false;
	bool bDisplayDpMax			= false;
	bool bDisplayPowerMax		= false;

	// Force filling list with all possible smart control valve.
	if( NULL == pHMSmartControlValve->GetpSelBodyMaterial() || NULL == pHMSmartControlValve->GetpSelConnection() || NULL == pHMSmartControlValve->GetpSelPN() ) 
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		m_pHM->ComputeAll();
	}
	
	if( NULL == pHMSmartControlValve->GetpSelBodyMaterial() || NULL == pHMSmartControlValve->GetpSelConnection() || NULL == pHMSmartControlValve->GetpSelPN() )
	{
		return;
	}
	
	std::multimap <int, CSmartControlValveData > *pMapSmartControlValve = pHMSmartControlValve->GetpMapSmartControlValve();
	std::multimap <int, CSmartControlValveData >::iterator ItMapSmartControlValve = pMapSmartControlValve->end();
	
	if( 0 == pMapSmartControlValve->size() )
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValve );
		m_pHM->ComputeAll();
	}

	// No possible choice
	if( 0 == pMapSmartControlValve->size() ) 
	{
		return;
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( lLast - 1, lLast - 1 );
	m_pCB->SetnRows( 7 );

	// Set columns title.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
	
	CString str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVENAME );
	m_pCB->SetRowParameters( lName, -1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVESIZE );
	m_pCB->SetRowParameters( lSize, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVEBODYMATERIAL );
	m_pCB->SetRowParameters( lBodyMaterial, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVECONNECT );
	m_pCB->SetRowParameters( lConnection, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVEPN );
	m_pCB->SetRowParameters( lPN, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVEQMAX );
	m_pCB->SetRowParameters( lFlowMax, 1, str, NULL );

	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVEPOWERMAX );
	m_pCB->SetRowParameters( lPowerMax, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTCONTROLVALVEDPMIN );
	m_pCB->SetRowParameters( lDpMin, 1, str, NULL );
		
	// Border.
	m_pCB->SetCellBorder( lName, 1, true, SS_BORDERTYPE_RIGHT );
	
	for( int i = lSize; i < lLast - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( lLast - 1, 1, true, SS_BORDERTYPE_LEFT );
	
	// Units.
	TCHAR unitname[_MAXCHARS];
	m_pCB->SetRowParameters( lName, -1, L"", NULL );
	m_pCB->SetRowParameters( lSize, 2, L"", NULL );
	m_pCB->SetRowParameters( lBodyMaterial, 2, L"", NULL );
	m_pCB->SetRowParameters( lConnection, 2, L"", NULL );
	m_pCB->SetRowParameters( lPN, 2, L"", NULL );

	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	m_pCB->SetRowParameters( lFlowMax, 2, unitname, NULL );

	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_TH_POWER ), unitname );
	m_pCB->SetRowParameters( lPowerMax, 2, unitname, NULL );

	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	m_pCB->SetRowParameters( lDpMin, 2, unitname, NULL );
	
	// Border.
	m_pCB->SetCellBorder( lName, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );
	
	for( int i = lSize; i < lLast - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( lLast - 1, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
	
	// Freeze row title.
	m_pCB->SetFreeze( 0 , 2 );
	
	// Fill rows.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	long lSelectedRow = 0;
	long lHighLightRow = 0;
	
	for( ItMapSmartControlValve = pMapSmartControlValve->begin(); pMapSmartControlValve->end() != ItMapSmartControlValve; ++ItMapSmartControlValve )
	{
		CDB_SmartControlValve *pclSmartControlValve = ItMapSmartControlValve->second.GetpSmartControlValve();

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

		// HYS-1221 : To show deleted products in red color in products list smart control valve.
		if( true == pclSmartControlValve->IsDeleted() )
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		long lRow = m_pCB->SetRowParameters( lName, -1, pclSmartControlValve->GetName(), (LPARAM) pclSmartControlValve );

		if( true == pHMSmartControlValve->IsBestSmartControlValve( pclSmartControlValve) )
		{
			lHighLightRow = lRow;
		}
		
		if( pHMSmartControlValve->GetIDPtr().MP == pclSmartControlValve )
		{
			lSelectedRow = lRow;
		}

		// Display the "Body material" column if not all the smart control valve have the same one.
		if( NULL != pHMSmartControlValve->GetIDPtr().MP && 0 != IDcmp( ( (CDB_SmartControlValve *)( pHMSmartControlValve->GetIDPtr().MP ) )->GetBodyMaterialID(), pclSmartControlValve->GetBodyMaterialID() ) )
		{
			bDisplayBodyMaterial = true;
		}

		// Display the "Connection" column if not all the smart control valve have the same one.
		if( NULL != pHMSmartControlValve->GetIDPtr().MP && 0 != IDcmp( ( (CDB_SmartControlValve *)( pHMSmartControlValve->GetIDPtr().MP ) )->GetConnectID(), pclSmartControlValve->GetConnectID() ) )
		{
			bDisplayConnect = true;
		}

		// Display the "PN" column if not all the smart control valve have the same one.
		if( NULL != pHMSmartControlValve->GetIDPtr().MP && 0 != IDcmp( ( (CDB_SmartControlValve *)( pHMSmartControlValve->GetIDPtr().MP ) )->GetPNID(), pclSmartControlValve->GetPNID() ) )
		{
			bDisplayPN = true;
		}

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lSize, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lBodyMaterial, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetBodyMaterialIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lConnection, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lPN, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetPNIDPtr().MP ) )->GetString(), NULL );
				
		// Flow max.
		m_pCB->SetRowParameters( lFlowMax, lRow, WriteCUDouble( _U_FLOW, ItMapSmartControlValve->second.GetQMax() ), NULL );

		if( SmartValveControlMode::SCVCM_Power == pHMSmartControlValve->GetControlMode() )
		{
			// Power max.
			m_pCB->SetRowParameters( lPowerMax, lRow, WriteCUDouble( _U_TH_POWER, ItMapSmartControlValve->second.GetPowerMax() ), NULL );
		}
		
		// HYS-1914: Dp min is the minimum pressure drop when the valve is fully open (Computed with design flow and Kvs).
		CString str = GetDashDotDash();
		CWaterChar *pclWaterChar = m_pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartControlValve );
		double dDpMin = -1.0;

		if( NULL != pclWaterChar )
		{
			dDpMin = ItMapSmartControlValve->second.GetpSmartControlValve()->GetDpMin( m_pHM->GetQ(), pclWaterChar->GetDens() );

			if( dDpMin > 0.0 )
			{
				str = WriteCUDouble( _U_DIFFPRESS, dDpMin );
			}
		}
		
		m_pCB->SetRowParameters( lDpMin, lRow, str , NULL );
		
		// Border.
		m_pCB->SetCellBorder( lName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i =lSize; i < lLast - 1; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
		
		m_pCB->SetCellBorder( lLast - 1, lRow, true, SS_BORDERTYPE_LEFT );
	
	}
	
	m_pCB->SetBackColor( 0, lHighLightRow, m_pCB->GetMaxCols(), lHighLightRow, _SOFTLIGHTGREEN );

	// Current existing smart control valve doesn't exist into the list, add it.
	if( 0 == lSelectedRow && NULL != pHMSmartControlValve->GetIDPtr().MP )
	{
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
		CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( pHMSmartControlValve->GetIDPtr().MP );
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
		long lRow = m_pCB->SetRowParameters( lName, -1, pclSmartControlValve->GetName(), (LPARAM)pclSmartControlValve );
		
		lSelectedRow = lRow;
		
		if( pHMSmartControlValve->GetIDPtr().MP == pclSmartControlValve )
		{
			lSelectedRow = lRow;
		}
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lSize, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lBodyMaterial, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetBodyMaterialIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lConnection, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lPN, lRow, ( (CDB_StringID *)( pclSmartControlValve->GetPNIDPtr().MP ) )->GetString(), NULL );
				
		// Flow max.
		m_pCB->SetRowParameters( lFlowMax, lRow, WriteCUDouble( _U_DIFFPRESS, pclSmartControlValve->GetQNom() ), NULL );

		if( SmartValveControlMode::SCVCM_Power == pHMSmartControlValve->GetControlMode() )
		{
			// Power max.
			m_pCB->SetRowParameters( lPowerMax, lRow, WriteCUDouble( _U_DIFFPRESS, pclSmartControlValve->GetPNom() ), NULL );
		}
		
		CWaterChar *pclWaterChar = m_pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartControlValve );
		ASSERT( NULL != pclWaterChar );

		if( NULL != pclWaterChar )
		{
			double dDp = CalcDp( pHMSmartControlValve->GetQ(), pclSmartControlValve->GetKvs(), pclWaterChar->GetDens() );
			m_pCB->SetRowParameters( lDpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDp ), NULL );
		}
				
		// Border.
		m_pCB->SetCellBorder( lName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i = lSize; i < lLast; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
	}

	m_pCB->SelectRow( lSelectedRow );
	
	// Hide unnecessary columns.
	if( SmartValveControlMode::SCVCM_Flow == pHMSmartControlValve->GetControlMode() )
	{
		// Hide power max column.
		m_pCB->ShowCol( lPowerMax, FALSE );
	}
	
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartControlValveBodyMaterial()
{
	CDS_HydroMod::CSmartControlValve *pclHMSmartControlValve = m_pHM->GetpSmartControlValve();

	if( NULL == pclHMSmartControlValve )
	{
		return;
	}
	
	std::map<int, CDB_StringID *> mapStrID;
	std::map<int, CDB_StringID *>::iterator It;
	int iSize = pclHMSmartControlValve->GetAvailableBodyMaterial( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_SmartControlValveBody;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpSmartControlValve()->GetpSelBodyMaterial() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpSmartControlValve()->GetpSelBodyMaterial()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartControlValveConnect()
{
	CDS_HydroMod::CSmartControlValve *pclHMSmartControlValve = m_pHM->GetpSmartControlValve();

	if( NULL == pclHMSmartControlValve )
	{
		return;
	}
	
	std::map<int, CDB_StringID *> mapStrID;
	std::map<int, CDB_StringID *>::iterator It;
	int iSize = pclHMSmartControlValve->GetAvailableConnection( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_SmartControlValveConnection;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpSmartControlValve()->GetpSelConnection() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpSmartControlValve()->GetpSelConnection()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartControlValvePN()
{
	CDS_HydroMod::CSmartControlValve *pclHMSmartControlValve = m_pHM->GetpSmartControlValve();

	if( NULL == pclHMSmartControlValve )
	{
		return;
	}
	
	std::map<int, CDB_StringID *> mapStrID;
	std::map<int, CDB_StringID *>::iterator It;
	int iSize = pclHMSmartControlValve->GetAvailablePN( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_SmartControlValvePN;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpSmartControlValve()->GetpSelPN() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpSmartControlValve()->GetpSelPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartDpC()
{
	CDS_HydroMod::CSmartDpC *pHMSmartDpC = m_pHM->GetpSmartDpC();

	if( NULL == pHMSmartDpC )
	{
		return;
	}

	// Variables
	long lName					= 1;
	long lSize					= 2;
	long lBodyMaterial			= 3;
	long lConnection			= 4;
	long lPN					= 5;
	long lDpMin					= 6;
	long lLast					= 7;
	bool bDisplayBodyMaterial	= false;
	bool bDisplayConnect		= false;
	bool bDisplayPN				= false;
	bool bDisplayDpMax			= false;

	// Force filling list with all possible smart differential pressure controller.
	if( NULL == pHMSmartDpC->GetpSelBodyMaterial() || NULL == pHMSmartDpC->GetpSelConnection() || NULL == pHMSmartDpC->GetpSelPN() ) 
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		m_pHM->ComputeAll();
	}
	
	if( NULL == pHMSmartDpC->GetpSelBodyMaterial() || NULL == pHMSmartDpC->GetpSelConnection() || NULL == pHMSmartDpC->GetpSelPN() )
	{
		return;
	}
	
	std::multimap <int, CSmartDDpCData > *pMapSmartDpC = pHMSmartDpC->GetpMapSmartDpC();
	std::multimap <int, CSmartDDpCData >::iterator ItMapSmartDpC = pMapSmartDpC->end();
	
	if( 0 == pMapSmartDpC->size() )
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartDpC );
		m_pHM->ComputeAll();
	}

	// No possible choice
	if( 0 == pMapSmartDpC->size() ) 
	{
		return;
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( lLast - 1, lLast - 1 );
	m_pCB->SetnRows( 7 );

	// Set columns title.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
	
	CString str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTDPCNAME );
	m_pCB->SetRowParameters( lName, -1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTDPCSIZE );
	m_pCB->SetRowParameters( lSize, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTDPCBODYMATERIAL );
	m_pCB->SetRowParameters( lBodyMaterial, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTDPCCONNECT );
	m_pCB->SetRowParameters( lConnection, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTDPCPN );
	m_pCB->SetRowParameters( lPN, 1, str, NULL );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_SMARTDPCDPMIN );
	m_pCB->SetRowParameters( lDpMin, 1, str, NULL );
		
	// Border.
	m_pCB->SetCellBorder( lName, 1, true, SS_BORDERTYPE_RIGHT );
	
	for( int i = lSize; i < lLast - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( lLast - 1, 1, true, SS_BORDERTYPE_LEFT );
	
	// Units.
	TCHAR unitname[_MAXCHARS];
	m_pCB->SetRowParameters( lName, -1, L"", NULL );
	m_pCB->SetRowParameters( lSize, 2, L"", NULL );
	m_pCB->SetRowParameters( lBodyMaterial, 2, L"", NULL );
	m_pCB->SetRowParameters( lConnection, 2, L"", NULL );
	m_pCB->SetRowParameters( lPN, 2, L"", NULL );

	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	m_pCB->SetRowParameters( lDpMin, 2, unitname, NULL );
	
	// Border.
	m_pCB->SetCellBorder( lName, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );
	
	for( int i = lSize; i < lLast - 1; i++ )
	{
		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	m_pCB->SetCellBorder( lLast - 1, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
	
	// Freeze row title.
	m_pCB->SetFreeze( 0 , 2 );
	
	// Fill rows.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	long lSelectedRow = 0;
	long lHighLightRow = 0;
	
	for( ItMapSmartDpC = pMapSmartDpC->begin(); pMapSmartDpC->end() != ItMapSmartDpC; ++ItMapSmartDpC )
	{
		CDB_SmartControlValve *pclSmartDpC = ItMapSmartDpC->second.GetpSmartDpC();

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

		// HYS-1221 : To show deleted products in red color in products list smart control valve.
		if( true == pclSmartDpC->IsDeleted() )
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else
		{
			m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		long lRow = m_pCB->SetRowParameters( lName, -1, pclSmartDpC->GetName(), (LPARAM) pclSmartDpC );

		if( true == pHMSmartDpC->IsBestValve( pclSmartDpC) )
		{
			lHighLightRow = lRow;
		}
		
		if( pHMSmartDpC->GetIDPtr().MP == pclSmartDpC )
		{
			lSelectedRow = lRow;
		}

		// Display the "Body material" column if not all the smart control valve have the same one.
		if( NULL != pHMSmartDpC->GetIDPtr().MP && 0 != IDcmp( ( (CDB_SmartControlValve *)( pHMSmartDpC->GetIDPtr().MP ) )->GetBodyMaterialID(), pclSmartDpC->GetBodyMaterialID() ) )
		{
			bDisplayBodyMaterial = true;
		}

		// Display the "Connection" column if not all the smart control valve have the same one.
		if( NULL != pHMSmartDpC->GetIDPtr().MP && 0 != IDcmp( ( (CDB_SmartControlValve *)( pHMSmartDpC->GetIDPtr().MP ) )->GetConnectID(), pclSmartDpC->GetConnectID() ) )
		{
			bDisplayConnect = true;
		}

		// Display the "PN" column if not all the smart control valve have the same one.
		if( NULL != pHMSmartDpC->GetIDPtr().MP && 0 != IDcmp( ( (CDB_SmartControlValve *)( pHMSmartDpC->GetIDPtr().MP ) )->GetPNID(), pclSmartDpC->GetPNID() ) )
		{
			bDisplayPN = true;
		}

		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lSize, lRow, ( (CDB_StringID *)( pclSmartDpC->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lBodyMaterial, lRow, ( (CDB_StringID *)( pclSmartDpC->GetBodyMaterialIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lConnection, lRow, ( (CDB_StringID *)( pclSmartDpC->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lPN, lRow, ( (CDB_StringID *)( pclSmartDpC->GetPNIDPtr().MP ) )->GetString(), NULL );
				
		// HYS-1914: Dp min is the minimum pressure drop when the valve is fully open (Computed with design flow and Kvs).
		CString str = GetDashDotDash();
		CWaterChar *pclWaterChar = m_pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartDpC );
		double dDpMin = -1.0;

		if( NULL != pclWaterChar )
		{
			dDpMin = ItMapSmartDpC->second.GetpSmartDpC()->GetDpMin( m_pHM->GetQ(), pclWaterChar->GetDens() );

			if( dDpMin > 0.0 )
			{
				str = WriteCUDouble( _U_DIFFPRESS, dDpMin );
			}
		}
		
		m_pCB->SetRowParameters( lDpMin, lRow, str , NULL );
		
		// Border.
		m_pCB->SetCellBorder( lName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i =lSize; i < lLast - 1; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
		
		m_pCB->SetCellBorder( lLast - 1, lRow, true, SS_BORDERTYPE_LEFT );
	
	}
	
	m_pCB->SetBackColor( 0, lHighLightRow, m_pCB->GetMaxCols(), lHighLightRow, _SOFTLIGHTGREEN );

	// Current existing smart differential pressure controller doesn't exist into the list, add it.
	if( 0 == lSelectedRow && NULL != pHMSmartDpC->GetIDPtr().MP )
	{
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
		CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( pHMSmartDpC->GetIDPtr().MP );
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
		long lRow = m_pCB->SetRowParameters( lName, -1, pclSmartDpC->GetName(), (LPARAM)pclSmartDpC );
		
		lSelectedRow = lRow;
		
		if( pHMSmartDpC->GetIDPtr().MP == pclSmartDpC )
		{
			lSelectedRow = lRow;
		}
		
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
		m_pCB->SetRowParameters( lSize, lRow, ( (CDB_StringID *)( pclSmartDpC->GetSizeIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lBodyMaterial, lRow, ( (CDB_StringID *)( pclSmartDpC->GetBodyMaterialIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lConnection, lRow, ( (CDB_StringID *)( pclSmartDpC->GetConnectIDPtr().MP ) )->GetString(), NULL );
		m_pCB->SetRowParameters( lPN, lRow, ( (CDB_StringID *)( pclSmartDpC->GetPNIDPtr().MP ) )->GetString(), NULL );
				
		CWaterChar *pclWaterChar = m_pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartDpC );
		ASSERT( NULL != pclWaterChar );

		if( NULL != pclWaterChar )
		{
			double dDp = CalcDp( pHMSmartDpC->GetQ(), pclSmartDpC->GetKvs(), pclWaterChar->GetDens() );
			m_pCB->SetRowParameters( lDpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDp ), NULL );
		}
				
		// Border.
		m_pCB->SetCellBorder( lName, lRow, true, SS_BORDERTYPE_RIGHT );
		
		for( int i = lSize; i < lLast; i++ )
		{
			m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
	}

	m_pCB->SelectRow( lSelectedRow );
	
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartDpCBodyMaterial()
{
	CDS_HydroMod::CSmartDpC *pclHMSmartDpC = m_pHM->GetpSmartDpC();

	if( NULL == pclHMSmartDpC )
	{
		return;
	}
	
	std::map<int, CDB_StringID *> mapStrID;
	std::map<int, CDB_StringID *>::iterator It;
	int iSize = pclHMSmartDpC->GetAvailableBodyMaterial( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_SmartDpCBody;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpSmartDpC()->GetpSelBodyMaterial() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpSmartDpC()->GetpSelBodyMaterial()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartDpCConnect()
{
	CDS_HydroMod::CSmartDpC *pclHMSmartDpC = m_pHM->GetpSmartDpC();

	if( NULL == pclHMSmartDpC )
	{
		return;
	}
	
	std::map<int, CDB_StringID *> mapStrID;
	std::map<int, CDB_StringID *>::iterator It;
	int iSize = pclHMSmartDpC->GetAvailableConnection( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_SmartDpCConnection;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpSmartDpC()->GetpSelConnection() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpSmartDpC()->GetpSelConnection()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBSmartDpCPN()
{
	CDS_HydroMod::CSmartDpC *pclHMSmartDpC = m_pHM->GetpSmartDpC();

	if( NULL == pclHMSmartDpC )
	{
		return;
	}
	
	std::map<int, CDB_StringID *> mapStrID;
	std::map<int, CDB_StringID *>::iterator It;
	int iSize = pclHMSmartDpC->GetAvailablePN( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_SmartDpCPN;

	// Fill spread combo box.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpSmartDpC()->GetpSelPN() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpSmartDpC()->GetpSelPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

long CDlgComboBoxHM::_FillRowCv( CDB_RegulatingValve *pRV, bool bKvs, bool bForSet )
{
	CString strValveName = pRV->GetName();

	if( true == bForSet )
	{
		strValveName += TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
	}
	
	// HYS-1221 : To show deleted products in red color in products list Cv, BCv, PIBCv.
	if( true == pRV->IsDeleted() )
	{
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	else
	{
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}

	long lRow = m_pCB->SetRowParameters( 1, -1, strValveName, (LPARAM)pRV );
	m_pCB->SetRowParameters( 2, lRow, ( (CDB_StringID *)( pRV->GetSizeIDPtr().MP ) )->GetString(), (LPARAM)pRV );
	double dQ = m_pHM->GetpCV()->GetQ();
	
	double dDp = 0.0;
	double dKvs = 0.0;
	const long lCvName			= 1;
	const long lCvSize			= 2;
	const long lCvType			= 3;
	const long lCvConnection	= 4;
	const long lCvVersion		= 5;
	const long lCvSetting		= 6;
	const long lCvKvs			= 7;
	const long lCvDp			= 8;		// HYS-1199: Can be 'Dpmin' for PIBCV.
	const long lCvDpFo			= 9;		// HYS-1199: Hidden in the case of PIBCV.
	const long lCvPN			= 10;
	const long lCvLast			= 11;
	bKvs = false;

	if( CDB_ControlProperties::eCVFUNC::ControlOnly == ( (CDB_CircuitScheme*)( m_pHM->GetSchemeIDPtr().MP ) )->GetCvFunc() )
	{
		if( NULL != m_pHM->GetpCV() && true == m_pHM->GetpCV()->IsTaCV() )
		{
			bKvs = true;
		}
	}

	// If curve exists, use it.
	if( NULL != pRV->GetValveCharacteristic() && true == pRV->GetValveCharacteristic()->HasKvCurve() )
	{
		dKvs = pRV->GetKvs();
		
		if( -1.0 == dKvs )
		{
			dKvs = 0.0;
		}
	}
	else
	{
		// Is it a control valve?
		CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pRV );
		
		if( NULL != pCV )
		{
			dKvs = pCV->GetKvs();
		}
		else
		{
			ASSERT( 0 );
		}
	}

	CWaterChar *pclWaterChar = m_pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	ASSERT( NULL != pclWaterChar );

	double dRho = pclWaterChar->GetDens();
	double dNu = pclWaterChar->GetKinVisc();
	
	if( true == bKvs )
	{
		m_pCB->SetRowParameters( lCvKvs, lRow, WriteCUDouble( _C_KVCVCOEFF, dKvs ), NULL );	
	}

	dDp = CalcDp( dQ, dKvs, dRho );
	m_pCB->SetRowParameters( lCvDp, lRow, WriteCUDouble( _U_DIFFPRESS, dDp ), NULL );	
	
	if( NULL != m_pHM->GetpCV() && true == m_pHM->GetpCV()->IsTaCV() )
	{
		// Preset & PressetPT.
		if( CDB_ControlProperties::ControlOnly != ( (CDB_CircuitScheme *)( m_pHM->GetSchemeIDPtr().MP ) )->GetCvFunc() )
		{
			// Set type, connection and version.
			m_pCB->SetRowParameters( lCvType, lRow, pRV->GetType(), NULL );
			m_pCB->SetRowParameters( lCvConnection, lRow, pRV->GetConnect(), NULL );
			m_pCB->SetRowParameters( lCvVersion, lRow, pRV->GetVersion(), NULL );

			double dh = 0.0;
			double dCvDp = 0.0;

			if( eb3True == m_pHM->GetpCV()->IsPICV() )
			{
				// HYS-1199: For PIBCV we show the 'Dpmin' instead of 'Dp'.
				CDB_PIControlValve *pclPIBCV = dynamic_cast<CDB_PIControlValve *>( pRV );
				ASSERT( NULL != pclPIBCV );

				dCvDp = pclPIBCV->GetDpmin( dQ, dRho );
			}
			else
			{
				dCvDp = m_pHM->GetpCV()->GetDp();
			}

			m_pCB->SetRowParameters( lCvDp, lRow, WriteCUDouble( _U_DIFFPRESS, dCvDp ), NULL );
			
			if( NULL != pRV->GetValveCharacteristic() && true == pRV->GetValveCharacteristic()->HasKvCurve() )
			{					
				CDB_ValveCharacteristic *pclValveCharacteristic = pRV->GetValveCharacteristic();
				double dKv = CalcKv( dQ, dCvDp, dRho );
				dh = pclValveCharacteristic->GetOpening( dKv );

				CString str = ( dh > 0.0 ) ? pclValveCharacteristic->GetSettingString( dh ) : _T("-");

				m_pCB->SetRowParameters( lCvSetting, lRow, str, NULL );	
				
				if( eb3True != m_pHM->GetpCV()->IsPICV() )
				{
					double dDp = pclValveCharacteristic->GetDpFullOpening( dQ, dRho, dNu);

					if( dDp >= 0.0 )
					{
						m_pCB->SetRowParameters( lCvDpFo, lRow, WriteCUDouble( _U_DIFFPRESS, dDp ), NULL );
					}
					else
					{
						m_pCB->SetRowParameters( lCvDpFo, lRow, _T("-"), NULL );
					}
				}
			}
			else
			{
				// In case the valve has no curve put "-" instead and show the correct Fc in case of PIControlValve

				m_pCB->SetRowParameters( lCvSetting, lRow, _T("-"), NULL );
				
				if( eb3True != m_pHM->GetpCV()->IsPICV() )
				{
					m_pCB->SetRowParameters( lCvDpFo, lRow, _T("-"), NULL );
				}
			}

			m_pCB->SetRowParameters( lCvPN, lRow, ( (CDB_StringID *)( pRV->GetPNIDPtr().MP ) )->GetString(), NULL );
		}
	}
	
	for( int i = 2; i <m_pCB->GetMaxCols(); i++ )
	{
		m_pCB->SetCellBorder( i, lRow, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
	}
	
	return lRow;
}

void CDlgComboBoxHM::_ShowCBCv()
{
	// HYS-1199: No more 'Fc' column for the PIBCV.

	// Variables
	const long lCvName			= 1;
	const long lCvSize			= 2;
	const long lCvType			= 3;
	const long lCvConnection	= 4;
	const long lCvVersion		= 5;
	const long lCvSetting		= 6;
	const long lCvKvs			= 7;
	const long lCvDp			= 8;
	const long lCvDpFo			= 9;
	const long lCvPN			= 10;
	const long lCvLast			= 11;
	bool bDisplayType	 = false;
	bool bDisplayConnect = false;
	bool bDisplayVersion = false;
	bool bKvsDisplayed = false;
	bool bDisplaySetting = false;
	bool bDisplayDpMin = false;
	bool bDisplayDpFO = false;
	bool bDisplayPN = false;
	CDB_ControlProperties::eCVFUNC eCurrentFunc = ( (CDB_CircuitScheme*)( m_pHM->GetSchemeIDPtr().MP ))->GetCvFunc();

	//	Control Only
	//Col 1: Name; Col 2: Size; Col 3: Dp 
	// Presetable
	//Col 1: Name; Col 2: Size;  Col 3: Setting; Col 4: Dp; Col 5: Dp at full opening
	//si on n emontre pas le Kvs pour les CV TA preset (KV reduit), on ne doit pas le montrer pour les control only nonplus 
	//(sauf si évidemment plus tard il y en a qui seront prop.)
	//Si +tard, vanne TA Prop, autorité à afficher en effet!
	// Pour le moment
	//Col 1 Name; Col 2 Size; Col 3 Dp pour control only
	//Col 1: Name; Col 2: Size;  Col 3: Setting; Col 4: Dp; Col 5: Dp at full opening  pour preset
	
	CDS_HydroMod::CCv *pCV = m_pHM->GetpCV();
	ASSERT( NULL != pCV );

	if( NULL == pCV )
	{
		return;
	}

	// Determine if the Cv is a PICV.
	bool bIsPIBCV = false;
	CDB_PIControlValve *pclPIControlValve = dynamic_cast<CDB_PIControlValve *>( pCV->GetCvIDPtr().MP ) ;

	if( NULL != pclPIControlValve )
	{
		bIsPIBCV = true;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pCV->GetCvIDPtr().MP );
	
	if( NULL == pTAP )
	{
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
	}

	pTAP = dynamic_cast<CDB_TAProduct *>( pCV->GetCvIDPtr().MP );

	if( NULL == pTAP )
	{
		return;
	}
	
	// Get current selected pipe.
	CDB_Pipe *pPipe = (CDB_Pipe *)( m_pHM->GetpPipe( CDS_HydroMod::eHMObj::eCV )->GetIDPtr().MP );
	
	if( NULL == pPipe )
	{
		return; // No pipe selected
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Find DN field based on Pipe size and SizeBelow and Size Above from technical parameters
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );
	int iDN15 = m_pclTableDN->GetSize( _T("DN_15") );
	int iDNMax = __max( iDN + m_pTechParam->GetSizeShiftAbove(), iDN15 );
	int iDNMin = __max( iDN + m_pTechParam->GetSizeShiftBelow(), 0 );
	
	CDB_CircuitScheme *pSch = (CDB_CircuitScheme *)( m_pHM->GetSchemeIDPtr().MP );
	ASSERT( NULL != pSch );

	CDB_CircSchemeCateg *pSchcat = (CDB_CircSchemeCateg *)( pSch->GetSchemeCategIDPtr().MP );
	ASSERT( NULL != pSchcat );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();	
	
	m_pCB->SetRedraw( FALSE );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnRows( 5 );	

	m_pCB->SetnCols( lCvLast - 1, lCvLast - 1 );

	// Fill spread ComboBox.
	// Two cases with and without presetting.
	if( CDB_ControlProperties::ControlOnly == eCurrentFunc )
	{
		if( NULL != m_pHM->GetpCV() && true == m_pHM->GetpCV()->IsTaCV() )
		{
			bKvsDisplayed = true;
		}
		
		_string str; 
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVTANAME );
		m_pCB->SetRowParameters( lCvName, -1, str.c_str(), NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVSIZE );
		m_pCB->SetRowParameters( lCvSize, 1, str.c_str(), NULL );
		
		if( true == bKvsDisplayed )
		{
			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_KVS );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CV );
			}
			
			m_pCB->SetRowParameters( lCvKvs, 1, str.c_str(), NULL );
		}	
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVDP );
		m_pCB->SetRowParameters( lCvDp, 1, str.c_str(), NULL );

		str = TASApp.LoadLocalizedString( IDS_PN );
		m_pCB->SetRowParameters( lCvPN, 1, str.c_str(), NULL );

		// Border.
		m_pCB->SetCellBorder( 1, 1, true, SS_BORDERTYPE_RIGHT );
		int i;
		
		for( i = lCvName + 1; i < lCvLast - 1; i++ )
		{
			m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}

		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_LEFT );
		
		// Units.
		TCHAR unitname[_MAXCHARS];
		m_pCB->SetRowParameters( lCvName, -1, _T(""), NULL );
		m_pCB->SetRowParameters( lCvSize, 2, _T(""), NULL );
		
		if( true == bKvsDisplayed )
		{
			m_pCB->SetRowParameters( lCvKvs, 2, _T(""), NULL );
		}

		GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
		m_pCB->SetRowParameters( lCvDp, 2, unitname, NULL );

		m_pCB->SetRowParameters( lCvPN, 2, _T(""), NULL );

		m_pCB->SetCellBorder( 1, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );
		
		for( i = lCvName + 1; i < lCvLast - 1; i++ )
		{
			m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}

		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
		
		// Freeze row title.
		m_pCB->SetFreeze( 0, 2 );
	}
	else
	{
		// Two cases exist : 1 for control valves
		//					 2 for Pressure independent control valves

		// HYS-1199: Col 7 becomes Dpmin for PIBCV.
		//           Col 8 is hidden for PIBCV: No more 'Fc' shown and there is no 'Dp 100%' for PIBCV.
		//Col 1: Name; Col 2: Size;  Col 3: Type; Col 4: Connection; Col 5: Version; Col 6: Setting; Col 7: Dp; Col 8: 'Dp 100%'
		bDisplayDpFO = true;
		m_pCB->SetnCols( lCvLast - 1, lCvLast - 1 );
		
		CString str; 
		m_pCB->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		m_pCB->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_LIGHTGRAY );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVTANAME );
		m_pCB->SetRowParameters( lCvName, -1, str, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVSIZE );
		m_pCB->SetRowParameters( lCvSize, 1, str, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVTYPE );
		m_pCB->SetRowParameters( lCvType, 1, str, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVCONN );
		m_pCB->SetRowParameters( lCvConnection, 1, str, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVVERS );
		m_pCB->SetRowParameters( lCvVersion, 1, str, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVPRESET );
		m_pCB->SetRowParameters( lCvSetting, 1, str, NULL );
		
		if( true == bIsPIBCV )
		{
			str = TASApp.LoadLocalizedString( IDS_DPMIN );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVDP );
		}

		m_pCB->SetRowParameters( lCvDp, 1, str, NULL );
		
		if( true == bIsPIBCV )
		{
			// HYS-1199: No more 'Fc' (Or 'Dp 100%') column for PIBCV.
			m_pCB->ShowCol( lCvDpFo, FALSE );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_CVDPFULLOPEN );
			m_pCB->SetRowParameters( lCvDpFo, 1, str, NULL );
		}

		str = TASApp.LoadLocalizedString( IDS_PN );
		m_pCB->SetRowParameters( lCvPN, 1, str, NULL );
		
		// Border.
		m_pCB->SetCellBorder( lCvName, 1, true, SS_BORDERTYPE_RIGHT );
		int i;
		
		for( i = lCvName + 1; i < lCvLast - 1; i++ )
		{
			m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
		
		m_pCB->SetCellBorder( i, 1, true, SS_BORDERTYPE_LEFT );
		
		// Units.
		TCHAR unitname[_MAXCHARS];
		m_pCB->SetRowParameters( lCvName, -1, L"", NULL );
		m_pCB->SetRowParameters( lCvType, 2, L"", NULL );
		m_pCB->SetRowParameters( lCvConnection, 2, L"", NULL );
		m_pCB->SetRowParameters( lCvVersion, 2, L"", NULL );
		m_pCB->SetRowParameters( lCvSize, 2, L"", NULL );
		str = TASApp.LoadLocalizedString( IDS_TURNS );
		m_pCB->SetRowParameters( lCvSetting, 2, str, NULL );
		GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );

		// HYS-1199: can be 'Dp' or 'Dpmin' when PIBCV. The unit doesn't change.
		m_pCB->SetRowParameters( lCvDp, 2, unitname, NULL );

		// HYS-1199: this column is hidden in case of PIBCV.
		if( false == bIsPIBCV )
		{
			m_pCB->SetRowParameters( lCvDpFo, 2, unitname, NULL );
		}

		m_pCB->SetRowParameters( lCvPN, 2, L"", NULL );

		m_pCB->SetCellBorder( lCvName, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT );
		
		for( i = lCvName + 1; i < lCvLast - 1; i++ )
		{
			m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT );
		}
		
		m_pCB->SetCellBorder( i, 2, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_LEFT );
		
		// Freeze row title.
		m_pCB->SetFreeze( 0, 2 );
	}
	
	// Fill rows.
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	long lSelectedRow = 0;
	long lHighLightRow = 0;
	
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	
	if( true == bIsPIBCV )
	{
		// HYS-1199: no 'Dp 100%' column for PIBCV.
		bDisplayDpFO = false;

		std::multimap <double,CDB_PIControlValve *> CvMap;
		std::multimap <double,CDB_PIControlValve *>::const_iterator it;
		
		m_pHM->GetpCV()->SelectBestPICV( 0, &CvMap );
		bool bForSet = ( eb3True == m_pHM->GetpCV()->GetCVSelectedAsaPackage() ) ? true : false;

		for( it = CvMap.begin(); it != CvMap.end(); ++it )
		{
			CDB_PIControlValve *pclPIControlValve = (CDB_PIControlValve *)( it->second->GetIDPtr().MP );
			ASSERT( NULL != pclPIControlValve );

			// In case it's presetting or PressetPT, Display Type, Connection, Version if at least one valve is different
			if( CDB_ControlProperties::ControlOnly != eCurrentFunc )
			{
				if( false == bDisplayType && 0 != IDcmp (pTAP->GetTypeID(), pclPIControlValve->GetTypeID() ) )
				{
					bDisplayType = true;
				}
				
				if( false == bDisplayConnect && 0 != IDcmp( pTAP->GetConnectID(), pclPIControlValve->GetConnectID() ) )
				{
					bDisplayConnect = true;
				}
				
				if( false == bDisplayVersion && 0 != IDcmp( pTAP->GetVersionID(), pclPIControlValve->GetVersionID() ) )
				{
					bDisplayVersion = true;
				}

				if( false == bDisplayPN && 0 != IDcmp( pTAP->GetPNIDPtr().ID, pclPIControlValve->GetPNIDPtr().ID ) )
				{
					bDisplayPN = true;
				}
			}

			// Verify the valve has the same Connection, Version and PN.
			long lRow = _FillRowCv( pclPIControlValve, bKvsDisplayed, bForSet );

			if( true == pCV->IsBestCV( pclPIControlValve->GetIDPtr().MP ) )
			{
				lHighLightRow = lRow;
			}
			
			if( pCV->GetCvIDPtr().MP == pclPIControlValve->GetIDPtr().MP )
			{
				lSelectedRow = lRow;
			}

			CWaterChar *pclWaterChar = m_pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
			ASSERT( NULL != pclWaterChar );

			if( -1.0 == pclPIControlValve->GetDpmin( m_pHM->GetpCV()->GetQ(), pclWaterChar->GetDens() ) )
			{
				bDisplayDpMin = true;
			}
		}
	}
	else
	{
		std::multimap <int, CCVData > *pMainValveMap = m_pHM->GetpCV()->GetpMapCV();
		std::multimap <int, CCVData >::iterator ItMainValveMap;
		
		// When MainValveMap is empty force filling list with all possible BV.
		if( 0 == pMainValveMap->size() ) 
		{
			m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
			m_pHM->ComputeAll();
		}

		// No possible choice.
		if( 0 == pMainValveMap->size() ) 
		{
			return;
		}

		bool bForSet = ( eb3True == m_pHM->GetpCV()->GetCVSelectedAsaPackage() ) ? true : false;
		
		for (ItMainValveMap = pMainValveMap->begin(); pMainValveMap->end() != ItMainValveMap; ++ItMainValveMap)
		{
			CDB_ControlValve *pCtrlV = ItMainValveMap->second.GetpCV();
			
			if( NULL == pCtrlV )
			{
				continue;
			}

			// In case it's presetting or PressetPT, Display Type, Connection, Version if at least one valve is different
			if( CDB_ControlProperties::ControlOnly != eCurrentFunc )
			{
				if( false == bDisplayType && 0 != IDcmp( pTAP->GetTypeID(), pCtrlV->GetTypeID() ) )
				{
					bDisplayType = true;
				}
			
				if( false == bDisplayConnect && 0 != IDcmp( pTAP->GetConnectID(), pCtrlV->GetConnectID() ) )
				{
					bDisplayConnect = true;
				}
				
				if( false == bDisplayVersion && IDcmp( pTAP->GetVersionID(), pCtrlV->GetVersionID() ) )
				{
					bDisplayVersion = true;
				}

				if( false == bDisplayPN && IDcmp( pTAP->GetPNIDPtr().ID, pCtrlV->GetPNIDPtr().ID ) )
				{
					bDisplayPN = true;
				}
			}

			// Verify the valve has the same Connection, Version and PN.
			long lRow = _FillRowCv( pCtrlV, bKvsDisplayed, bForSet );

			if( true == pCV->IsBestCV( pCtrlV->GetIDPtr().MP ) )
			{
				lHighLightRow = lRow;
			}
			
			if( pCV->GetCvIDPtr().MP == pCtrlV->GetIDPtr().MP )
			{
				lSelectedRow = lRow;
			}
		}
	}

	m_pCB->SetBackColor(0,lHighLightRow,m_pCB->GetMaxCols(),lHighLightRow,_SOFTLIGHTGREEN);
	m_pCB->SelectRow( lSelectedRow );

	// Hide unnecessary columns.
	if( false == bDisplayType )
	{
		_HideUnnecessaryCol( lCvType, lCvType );
	}
	
	if( false == bDisplayConnect )
	{
		_HideUnnecessaryCol( lCvConnection, lCvConnection );
	}
	
	if( false == bDisplayVersion )
	{
		_HideUnnecessaryCol( lCvVersion, lCvVersion );
	}
	
	if( false == bDisplaySetting )
	{
		_HideUnnecessaryCol( lCvSetting, lCvSetting );
	}
	
	if( false == bKvsDisplayed )
	{
		_HideUnnecessaryCol( lCvKvs, lCvKvs );
	}

	// HYS-1199: if we show PIBCV but there is no valve with a Dpmin defined we hide the 'Dpmin' column.
	if( true == bIsPIBCV && false == bDisplayDpMin )
	{
		_HideUnnecessaryCol( lCvDp, lCvDp );
	}

	if( false == bDisplayDpFO )
	{
		_HideUnnecessaryCol( lCvDpFo, lCvDpFo );
	}

	if( false == bDisplayPN )
	{
		_HideUnnecessaryCol( lCvPN, lCvPN );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBCvKvs()
{
	CDS_HydroMod::CCv *pCV = m_pHM->GetpCV();			//ASSERT( pCV );

	if( NULL == pCV )
	{
		return;
	}
	
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 2, 1 );
	m_pCB->SetnRows( 5 );
	
	CDB_MultiString *pRHSerie = NULL;

	if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		pRHSerie = (CDB_MultiString *)( m_pTADB->Get( _T("REYNARD_SERIES") ).MP );
	}
	else
	{
		pRHSerie = (CDB_MultiString *)( m_pTADB->Get( _T("CV_SERIES") ).MP );
	}
	ASSERT( NULL != pRHSerie );
	
	int iHLRow = INT_MAX;
	double dValue = pCV->GetBestKvs();
	dValue = CDimValue::SItoCU( _C_KVCVCOEFF, dValue );	
	LPARAM cvkvs = (LPARAM)( dValue * 100 + 0.5 );
	LPARAM lParam = 0;
	
	for( int i =0; i <pRHSerie->GetNumofLines(); i++ )
	{
		lParam = (LPARAM)( _tstof( pRHSerie->GetString( i ) ) * 100 + 0.5 );
		int j = m_pCB->SetRowParameters( 1, -1, CString( pRHSerie->GetString( i ) ), lParam );
		
		if( lParam == cvkvs )
		{
			iHLRow = j;
		}
	}
	
	if( iHLRow != INT_MAX )
	{
		m_pCB->SetBackColor(0,iHLRow,m_pCB->GetMaxCols(),iHLRow,_SOFTLIGHTGREEN);
	}

	dValue=pCV->GetKvs();
	dValue = CDimValue::SItoCU( _C_KVCVCOEFF, dValue );	
	
	if( 0.0 != dValue )
	{
		m_pCB->FindAndSelectRow( 1, (LPARAM)( dValue * 100 + 0.5 ) );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBCvConnect()
{
	// Get current selected pipe.
	CDB_Pipe *pPipe = (CDB_Pipe *)( m_pHM->GetpPipe( CDS_HydroMod::eHMObj::eCV )->GetIDPtr().MP );
	
	if( NULL == pPipe )
	{
		return; // No pipe selected
	}

	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = m_pHM->GetpCV()->GetAvailableConnections( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_CvConnection;
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );

	// Start filling row.
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
		
	if( NULL != m_pHM->GetpCV()->GetpSelCVConn() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpCV()->GetpSelCVConn()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBCvVersion()
{
	// Fill the connection combo according to selected type.
	if( NULL == m_pHM->GetpCV()->GetpSelCVConn() ) 
	{
		m_pHM->GetpCV()->SetpUserChoiceCVVers( NULL );
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = m_pHM->GetpCV()->GetAvailableVersions( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_eSCB = eSCB::SCB_CvVersion;
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );

	// Start filling row.
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpCV()->GetpSelCVVers() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpCV()->GetpSelCVVers()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBCvPN()
{
	// Fill the connection combo according to selected type.
	if( NULL == m_pHM->GetpCV()->GetpSelCVConn() || NULL == m_pHM->GetpCV()->GetpSelCVVers() ) 
	{
		m_pHM->GetpCV()->SetpUserChoiceCVPN( NULL );
		return;
	}
	
	std::map<int, CDB_StringID*> mapStrID;
	std::map<int, CDB_StringID*>::iterator It;
	int iSize = m_pHM->GetpCV()->GetAvailablePN( &mapStrID );
	
	if( iSize <= 0 )
	{
		return;	
	}

	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	CRankEx SortList;
	
	// Get current selected pipe.
	CDB_Pipe *pPipe = (CDB_Pipe *)( m_pHM->GetpPipe( CDS_HydroMod::eHMObj::eCV )->GetIDPtr().MP );
	
	if( NULL == pPipe )
	{
		return; // No pipe selected
	}

	m_eSCB = eSCB::SCB_CvPN;
	
	// Fill spread ComboBox.
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );
	
	// Start filling row.
	for( It = mapStrID.begin(); It != mapStrID.end(); ++It )
	{
		m_pCB->SetRowParameters( 1, -1, It->second->GetString(), (LPARAM)It->second );
	}
	
	if( NULL != m_pHM->GetpCV()->GetpSelCVPN() )
	{
		m_pCB->FindAndSelectRow( 1, m_pHM->GetpCV()->GetpSelCVPN()->GetString() );
	}

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBTUFlow()
{
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	SetLParam( (LPARAM)( m_pHM->GetpTermUnit()->GetQType() + 1 ) );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 2 );

	// Add 3 flow types.
	CString str = TASApp.LoadLocalizedString( IDS_DLGCBHM_TUFLOW );
	// Remark: '+1' to avoid NULL parameter.
	m_pCB->SetRowParameters( 1, -1, str, (LPARAM)CTermUnit::_QType::Q + 1 );
	str = TASApp.LoadLocalizedString( IDS_DLGCBHM_TUFLOWPDT );
	m_pCB->SetRowParameters( 1, -1, str, (LPARAM)CTermUnit::_QType::PdT + 1 );

	m_pCB->FindAndSelectRow( 1, (LPARAM)( m_pHM->GetpTermUnit()->GetQType() + 1 ) );

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowCBTUDp()
{
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}

	SetLParam( (LPARAM)m_pHM->GetpTermUnit()->GetDpType() );
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 4 );

	// Add 4 Dp types.
	CString str = TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDP );
	m_pCB->SetRowParameters( 1, -1, str, (LPARAM)CDS_HydroMod::eDpType::Dp );
	str = TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDPQREFDPREF );
	m_pCB->SetRowParameters( 1, -1, str, (LPARAM)CDS_HydroMod::eDpType::QDpref );
	str = TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDPKV );
	m_pCB->SetRowParameters( 1, -1, str, (LPARAM)CDS_HydroMod::eDpType::Kv );
	str = TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDPCV );
	m_pCB->SetRowParameters( 1, -1, str, (LPARAM)CDS_HydroMod::eDpType::Cv );

	m_pCB->FindAndSelectRow( 1, (LPARAM)m_pHM->GetpTermUnit()->GetDpType() );

	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_ShowList( long lColumn, long lRow )
{
	if( 0 == GetSafeHwnd() )
	{
		Create();
	}
	else
	{
		Refresh();
	}
	
	m_pCB->SetRedraw( false );
	m_pCB->Empty();
	m_pCB->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pCB->SetnCols( 1, 1 );
	m_pCB->SetnRows( 5 );

	CString str;
	LPARAM lParam;
	
	for( BOOL bContinue = m_pRankList->GetFirst( str, lParam ); TRUE == bContinue; bContinue = m_pRankList->GetNext( str, lParam ) )
	{
		m_pCB->SetRowParameters( 1, -1, str, lParam );	
	}
	
	m_pCB->FindAndSelectRow( 1, m_pMainSheet->GetCellText( lColumn, lRow ) );
	m_pCB->SetRedraw();
	m_pCB->Invalidate();
}

void CDlgComboBoxHM::_HideUnnecessaryCol( long lFromColumn, long lToColumn )
{
	for( long lLoopColumn = lFromColumn; lLoopColumn <= lToColumn; lLoopColumn++ )
	{
		CString str;
		bool bhide =true;

		for( long lLoopRow = 3; lLoopRow < m_pCB->GetMaxRows(); lLoopRow++ )
		{
			if( true == str.IsEmpty() )
			{
				str = m_pCB->GetCellText( lLoopColumn, lLoopRow );
			}

			if( str != m_pCB->GetCellText( lLoopColumn, lLoopRow ) )
			{
				bhide = false;
				break;
			}
		}
		
		if( true == bhide )
		{
			m_pCB->ShowCol( lLoopColumn, FALSE );
		}
	}
}

void CDlgComboBoxHM::_InitListStationActuatorType()
{
	m_pRankList->PurgeAll();

	CString str;
	LPARAM lParam = 0;
	CRank SortList;

	if( ( (CDS_HmHubStation *)m_pHM)->GetBalTypeID() == _T("RVTYPE_BVC") )
	{
		CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( ( (CDS_HmHubStation *)m_pHM )->GetTADBValveIDPtr().MP );
		
		if( NULL != pCV )
		{
			CTable *pRTab = dynamic_cast<CTable *>( pCV->GetActuatorGroupIDPtr().MP );
			
			if( NULL != pRTab )
			{
				for( IDPTR IDPtr = pRTab->GetFirst(); NULL != *IDPtr.ID; IDPtr = pRTab->GetNext() )
				{
					CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( IDPtr.MP );			ASSERT( pActr );
					m_pRankList->AddStrSort( pActr->GetName(), 1, (LPARAM)pActr );
				}
			}

			CString str = TASApp.LoadLocalizedString( IDS_SSHEETTAHUB_CVTYPENONE );
			m_pRankList->Add( str, 0, -1 );
		}
	}
	else
	{
		lParam = (LPARAM)0;
		str= _T("");
		TCHAR* endptr;
		double dKey = _tcstod( str, &endptr );
		m_pRankList->Add( str, dKey, lParam );
	}
}

void CDlgComboBoxHM::_InitListStationValveType()
{
	m_pRankList->PurgeAll();

	CString str;
	LPARAM lParam = 0;
	
	// Fill a temporary table with available type (not filtered out)
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("HUBSTAVALV_TAB" ) ).MP );
	ASSERT( NULL != pTab );

	CRank rkList( false );
	IDPTR IDPtrStaValv = _NULL_IDPTR;
	int i = 0;
	
	for( IDPtrStaValv = pTab->GetFirst(); NULL != *IDPtrStaValv.ID; IDPtrStaValv = pTab->GetNext() )
	{
		if( ( (CDB_HubStaValv *)IDPtrStaValv.MP )->GetSupplyOrReturn() != CDB_HubStaValv::eSupplyReturnValv::Return )
		{
			continue;
		}
		
		IDPTR IDPtr = ( (CDB_HubStaValv *)IDPtrStaValv.MP )->GetValveIDPtr();
		
		// NO_ID for instance.
		if( NULL == IDPtr.MP )
		{
			continue;
		}
		
		// Available for Hub ?
		if( false == IDPtr.MP->IsSelectable( true, true ) )
		{
			continue;
		}
		
		if( TRUE == rkList.IfExist( ( (CDB_TAProduct*)( IDPtr.MP ) )->GetTypeID() ) )
		{
			continue;
		}
		
		rkList.Add( ( (CDB_TAProduct*)( IDPtr.MP ) )->GetTypeID(), i++ );
	}

	pTab = (CTable*)( m_pTADB->Get( _T("STASUPRET_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); NULL != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		IDPTR IDPtrType = ( (CDB_HubSupRetLnk *)( IDPtr.MP ) )->GetValveTypeIDPtr();

		if( false == rkList.IfExist( IDPtrType.ID ) )
		{
			continue;
		}
		
		str = ( (CDB_StringID *)( IDPtrType.MP ) )->GetString();

		if( TRUE == m_pRankList->IfExist( str ) )
		{
			continue;
		}
		
		lParam = (LPARAM)( IDPtrType.MP );
		TCHAR *ptcEndptr = NULL;
		
		double dKey = _tcstod( ( (CDB_StringID*)( IDPtrType.MP ) )->GetIDstr(), &ptcEndptr );
		ASSERT( _T('\0') == *ptcEndptr );

		m_pRankList->Add( str, dKey, lParam );
	}

	rkList.PurgeAll();
}

void CDlgComboBoxHM::_InitListStationValveCtrlType()
{
	m_pRankList->PurgeAll();
	CString str;
	LPARAM lParam = 0;
	
	// Fill a temporary table with available type (not filtered out)
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("HUBSTAVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	IDPTR IDPtrStaValv = _NULL_IDPTR;
	int i = 0;
	
	for( IDPtrStaValv = pTab->GetFirst(); NULL != *IDPtrStaValv.ID; IDPtrStaValv = pTab->GetNext() )
	{
		if( ( (CDB_HubStaValv *)IDPtrStaValv.MP )->GetSupplyOrReturn() != CDB_HubStaValv::eSupplyReturnValv::Return )
		{
			continue;
		}
		
		IDPTR IDPtr = ( (CDB_HubStaValv *)IDPtrStaValv.MP )->GetValveIDPtr();
		
		// NO_ID for instance.
		if( NULL == IDPtr.MP )
		{
			continue;
		}
		
		// Available for Hub ?
		if( false == IDPtr.MP->IsSelectable( true, true ) )
		{
			continue;
		}
		
		CDB_RegulatingValve *pRV = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

		if( NULL == pRV )
		{
			continue;
		}
		
		if( NULL == pRV->GetCtrlProp() )
		{
			continue;
		}
		
		CString str = CDB_ControlProperties::GetCvCtrlTypeStr( pRV->GetCtrlProp()->GetCtrlType() ).c_str();
		
		if( TRUE == m_pRankList->IfExist( str ) )
		{
			continue;
		}
		
		m_pRankList->Add( str, pRV->GetCtrlProp()->GetCtrlType(), pRV->GetCtrlProp()->GetCtrlType() + 1 );
	}
}

void CDlgComboBoxHM::_InitListStationValveName()
{
	CString str;
	LPARAM lParam = 0;
	
	m_pRankList->PurgeAll();

	CTable *pTab = (CTable *)( m_pTADB->Get( _T("STASUPRET_TAB") ).MP );
	ASSERT(	NULL != pTab );
	
	CTable *pTabValve = (CTable *)( m_pTADB->Get( _T("HUBSTAVALV_TAB") ).MP );
	ASSERT( NULL != pTabValve );
	
	CDS_HmHubStation *m_pHubStation = (CDS_HmHubStation *)m_pHM;
	IDPTR IDPtrPairs = _NULL_IDPTR;
	CDB_HubSupRetLnk *pHubSR=NULL;
	
	// Scan pairs.
	for( IDPtrPairs = pTab->GetFirst(); _T('\0') != *IDPtrPairs.ID; IDPtrPairs = pTab->GetNext() )
	{
		pHubSR = (CDB_HubSupRetLnk *)( IDPtrPairs.MP );
		
		// For all valve with the same type.
		if( m_pHubStation->GetBalTypeID() == pHubSR->GetValveTypeID() )
		{
			IDPTR IDPtr = _NULL_IDPTR;
			
			for( IDPtr = pTabValve->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTabValve->GetNext() )
			{
				CDB_HubStaValv *pRetValv = (CDB_HubStaValv *)( IDPtr.MP );
				lParam = (LPARAM)( IDPtr.MP );
				
				// I keep valve with the same TAIS
				if( ( pRetValv->GetTAISValve() == pHubSR->GetTAISReturn() ) && ( pRetValv->GetSupplyOrReturn() == CDB_HubStaValv::eSupplyReturnValv::Return ) )
				{	
					// Verify valve availability
					CDB_Thing *pThing = (CDB_Thing *)( pRetValv->GetValveIDPtr().MP );
					ASSERT( NULL != pThing );

					if( NULL == pThing )
					{
						return;
					}

					if( false == pThing->IsSelectable( true, true ) )
					{
						continue;
					}

					CDS_HmHubStation *pHM = dynamic_cast<CDS_HmHubStation *>( m_pHM );
					CDB_RegulatingValve *pRV = dynamic_cast <CDB_RegulatingValve *>( pThing );
				
					if( NULL != pRV && NULL != pHM && NULL != pHM->GetpCV() )
					{
						if( false == pRV->GetCtrlProp()->CvCtrlTypeFits( pHM->GetpCV()->GetCtrlType() ) )
						{
							continue;
						}
					}

					str = ( (CDB_TAProduct *)( pRetValv->GetValveIDPtr().MP ) )->GetName();

					if( m_pRankList->IfExist( str ) )
					{
						continue;
					}
					
					TCHAR* endptr;
					double dKey = _tcstod( str, &endptr );
					m_pRankList->Add( str, dKey, lParam );
				}
			}	
		}
	}
}

void CDlgComboBoxHM::_InitListHubValveName()
{
	CString str;
	LPARAM lParam = 0;

	m_pRankList->PurgeAll();

	CTable *pTab = 	(CTable *)( m_pTADB->Get( _T("HUBSUPRET_TAB") ).MP );
	ASSERT( NULL != pTab );

	CTable *pTabValve =	(CTable *)( m_pTADB->Get( _T("HUBVALV_TAB" ) ).MP );
	ASSERT( NULL != pTabValve );

	CDS_HmHub *m_pHub = (CDS_HmHub *)m_pHM;
	IDPTR IDPtrPairs = _NULL_IDPTR;
	CDB_HubSupRetLnk *pHubSR = NULL;
	
	// Scan Pairs
	for( IDPtrPairs = pTab->GetFirst(); _T('\0') != *IDPtrPairs.ID; IDPtrPairs = pTab->GetNext() )
	{
		pHubSR = (CDB_HubSupRetLnk *)( IDPtrPairs.MP );
		
		// For all hub with the same type.
		if( m_pHub->GetBalTypeID() == pHubSR->GetValveTypeID() )
		{
			IDPTR IDPtr = _NULL_IDPTR;

			for( IDPtr = pTabValve->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTabValve->GetNext() )
			{
				CDB_HubValv *pRetValv = (CDB_HubValv *)( IDPtr.MP );
				lParam = (LPARAM)( IDPtr.MP );
				
				// I keep valve with the same TAIS.
				if( pRetValv->GetTAISPartner() == pHubSR->GetTAISReturn() && pRetValv->GetSupplyOrReturn() == CDB_HubValv::eSupplyReturnValv::Return )
				{
					// Verify valve availability
					CDB_Thing *pThing = (CDB_Thing *)( pRetValv->GetValveIDPtr().MP );
					ASSERT( NULL != pThing );

					if( NULL == pThing )
					{
						return;
					}
					
					if( false == pThing->IsSelectable( true, true ) )
					{
						continue;
					}
					
					str = ( (CDB_TAProduct *)( pRetValv->GetValveIDPtr().MP ) )->GetName();
					
					if( TRUE == m_pRankList->IfExist( str ) )
					{
						continue;
					}

					TCHAR *endptr;
					double dKey = _tcstod( str, &endptr );
					m_pRankList->Add( str, dKey, lParam );
				}
			}	
		}
	}
}

void CDlgComboBoxHM::_InitListHubDpCName()
{
	m_pRankList->PurgeAll();
	CString str;
	LPARAM lParam = 0;

	CTable *pTab = (CTable *)( m_pTADB->Get( _T("HUBSUPRET_TAB" ) ).MP );
	ASSERT( NULL != pTab );

	CTable *pTabValve =	(CTable *)( m_pTADB->Get(_T("HUBVALV_TAB") ).MP );
	ASSERT( NULL != pTabValve );

	CTable *pDpCTab = (CTable *)( m_pTADB->Get(_T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pDpCTab );	

	CDS_HmHub *m_pHub = (CDS_HmHub *)m_pHM;
	IDPTR IDPtrPairs = _NULL_IDPTR;
	CDB_HubSupRetLnk *pHubSR = NULL;
	
	// Scan pairs.
	for( IDPtrPairs = pTab->GetFirst(); _T('\0') != *IDPtrPairs.ID; IDPtrPairs = pTab->GetNext() )
	{
		pHubSR = (CDB_HubSupRetLnk *)( IDPtrPairs.MP );
		
		// For all with the same type.
		if( m_pHub->GetBalTypeID() == pHubSR->GetValveTypeID() )
		{
			IDPTR IDPtr = _NULL_IDPTR;

			for( IDPtr = pTabValve->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTabValve->GetNext() )
			{
				CDB_HubValv *pRetValv = (CDB_HubValv *)( IDPtr.MP );
				lParam = (LPARAM)( IDPtr.MP );
				
				// Keep valve with the same TAIS.
				if( pRetValv->GetTAISPartner() == pHubSR->GetTAISReturn() && pRetValv->GetSupplyOrReturn() == CDB_HubValv::eSupplyReturnValv::Return )
				{			
					// Verify valve availability.
					CDB_Thing *pThing = (CDB_Thing *)( pRetValv->GetValveIDPtr().MP );
					ASSERT( NULL != pThing );

					if( NULL == pThing )
					{
						return;
					}
					
					if( false == pThing->IsSelectable( true, true ) )
					{
						continue;
					}
					
					// Locate available DpC.
					CDB_DpController *pDpC = (CDB_DpController*)( pDpCTab->Get( pRetValv->GetValveIDPtr().ID ).MP );
					ASSERT( NULL != pDpC );

					str = ( NULL != pDpC ) ? pDpC->GetFormatedDplRange().c_str() : _T("");
					str.Format( pRetValv->GetName() + _T(" ") + str );
					
					// If already added skip it.
					if( TRUE == m_pRankList->IfExist( str ) )
					{
						continue;
					}
					
					TCHAR *endptr;
					double dKey = _tcstod( str, &endptr );
					m_pRankList->Add( str, dKey, lParam );
				}
			}	
		}
	}
}

void CDlgComboBoxHM::_InitListShutOffValve()
{
	_InitListHubValveName();
}
