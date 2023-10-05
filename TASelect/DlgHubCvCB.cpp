// DlgHubCvCB.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"

#include "Global.h"
#include "Utilities.h"
#include "databobj.h"
#include "HMInclude.h"

#include "DlgHubCvCB.h"


// CDlgHubCvCB dialog

IMPLEMENT_DYNAMIC( CDlgHubCvCB, CDlgSpreadCB )

CDlgHubCvCB::CDlgHubCvCB( CSSheet *pOwnerSSheet )
	: CDlgSpreadCB( IDD, pOwnerSSheet )
{
}

CDlgHubCvCB::~CDlgHubCvCB()
{
}

void CDlgHubCvCB::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_COMBOCV, m_ComboCv );
	DDX_Control( pDX, IDC_COMBOCTRLTYPE, m_ComboCvCtrlType );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
}

void CDlgHubCvCB::OpenDialogSCB( CDS_HydroMod *pHM, long col, long row )
{
	m_pHM = pHM;
	SetMainSheetPos(m_pMainSheet,col,row);
	if (GetSafeHwnd() == 0)
		Create();
	else
		SendMessage(WM_INITDIALOG);
}

void CDlgHubCvCB::CloseDialogCB()
{
	BeginWaitCursor();
	CString str;
	LPARAM lpCvType=m_ComboCv.GetItemData(m_ComboCv.GetCurSel());
	LPARAM lpCvCtrlType=m_ComboCvCtrlType.GetItemData(m_ComboCvCtrlType.GetCurSel());
	LPARAM lpCvConnect, lpCVVersion, lpCVPN;
	lpCvConnect=lpCVVersion=lpCVPN=0;
	if ((CDS_HmHubStation::eCVTAKV)lpCvType==CDS_HmHubStation::eCVTAKV::eTAValve)
	{
		if (m_ComboConnect.GetCount())
			lpCvConnect = m_ComboConnect.GetItemData(m_ComboConnect.GetCurSel());
		if (m_ComboVersion.GetCount())
			lpCVVersion = m_ComboVersion.GetItemData(m_ComboVersion.GetCurSel());
		if (m_ComboPN.GetCount())
			lpCVPN = m_ComboPN.GetItemData(m_ComboPN.GetCurSel());
	}
	((CDS_HmHubStation *)m_pHM)->SetControlValve((CDS_HmHubStation::eCVTAKV)lpCvType, (CDB_ControlProperties::CvCtrlType)lpCvCtrlType,
												(CDB_StringID*)lpCvConnect,(CDB_StringID*)lpCVVersion,(CDB_StringID*)lpCVPN);
	((CDS_HmHubStation *)m_pHM)->ComputeHM(CDS_HydroMod::eComputeHMEvent::eceResize);
	EndWaitCursor();
}

void CDlgHubCvCB::Refresh()
{
	CRect rect;
	GetClientRect(&rect);
	GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.MoveToXY(0,0);
	// Cell position in MainSheet
	CRect Srect = GetComboPos(rect);
	::SetWindowPos(this->m_hWnd,HWND_TOPMOST,Srect.left,Srect.top,rect.Width(),rect.Height(),SWP_SHOWWINDOW);
}

BEGIN_MESSAGE_MAP(CDlgHubCvCB, CDialog)
	// User Messages
	ON_MESSAGE(WM_USER_CLOSESPREADCOMBOBOX, OnCloseCB)
	ON_CBN_SELCHANGE(IDC_COMBOCV, &CDlgHubCvCB::OnCbnSelchangeCombocv)
	ON_CBN_SELCHANGE(IDC_COMBOCTRLTYPE, &CDlgHubCvCB::OnCbnSelchangeComboctrltype)
	ON_CBN_SELCHANGE(IDC_COMBOCONNECT, &CDlgHubCvCB::OnCbnSelchangeComboconnect)
	ON_CBN_SELCHANGE(IDC_COMBOVERSION, &CDlgHubCvCB::OnCbnSelchangeComboversion)
	ON_CBN_SELCHANGE(IDC_COMBOPN, &CDlgHubCvCB::OnCbnSelchangeCombopn)
	ON_WM_ACTIVATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CDlgHubCvCB message handlers
BOOL CDlgHubCvCB::OnInitDialog()
{
	CDlgSpreadCB::OnInitDialog();
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGHUBCVCB_CV);
	GetDlgItem(IDC_STATICCV)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGHUBCVCB_STATICCONTROLTYPE);
	GetDlgItem(IDC_STATICCONTROLTYPE)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGHUBCVCB_STATICCONNECT);
	GetDlgItem(IDC_STATICCONNECT)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGHUBCVCB_STATICVERSION);
	GetDlgItem(IDC_STATICVERSION)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGHUBCVCB_STATICPN);
	GetDlgItem(IDC_STATICPN)->SetWindowText(str);
	Refresh();
	FillComboCV();
	// If Cv already exist initialise combo with selected values
	if (m_pHM->IsCvExist())
	{
		for (int i=0; i<m_ComboCv.GetCount(); i++)
			if ((CDS_HmHubStation::eCVTAKV)m_ComboCv.GetItemData(i) == ((CDS_HmHubStation*)m_pHM)->GetCvTaKv())
			{
				m_ComboCv.SetCurSel(i);
				break;		
			};
		OnCbnSelchangeCombocv();	
		for (int i=0; i<m_ComboCvCtrlType.GetCount(); i++)
			if ((CDB_ControlProperties::CvCtrlType)m_ComboCvCtrlType.GetItemData(i) == m_pHM->GetpCV()->GetCtrlType())
			{
				m_ComboCvCtrlType.SetCurSel(i);
				break;		
			};
		OnCbnSelchangeComboctrltype();
		if (m_pHM->GetpCV()->IsTaCV())
		{
			for (int i=0; i<m_ComboConnect.GetCount(); i++)
				if ((CDB_StringID *)m_ComboConnect.GetItemData(i) == m_pHM->GetpCV()->GetpSelCVConn())
				{
					m_ComboConnect.SetCurSel(i);
					break;		
				};
			OnCbnSelchangeComboconnect();
			for (int i=0; i<m_ComboVersion.GetCount(); i++)
				if ((CDB_StringID *)m_ComboVersion.GetItemData(i) == m_pHM->GetpCV()->GetpSelCVVers())
				{
					m_ComboVersion.SetCurSel(i);
					break;		
				};
			OnCbnSelchangeComboversion();
			for (int i=0; i<m_ComboPN.GetCount(); i++)
				if ((CDB_StringID *)m_ComboPN.GetItemData(i) == m_pHM->GetpCV()->GetpSelCVPN())
				{
					m_ComboPN.SetCurSel(i);
					break;		
				};
			OnCbnSelchangeCombopn();
		};
	}
	return TRUE;  
}

void CDlgHubCvCB::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_INACTIVE)
	{
		CloseDialogCB();
	}
	__super::OnActivate(nState, pWndOther, bMinimized);
}

void CDlgHubCvCB::FillComboCV()
{
	m_ComboCv.ResetContent();
	CDS_HmHubStation *pHS = (CDS_HmHubStation *)m_pHM;
	CRankEx rkEx;
	int iCvTA = GetpTADB()->GetTaCVList(&rkEx,CTADatabase::eForHMCv, true, CDB_ControlProperties::CV2W3W::CV2W,
										L"",L"",L"",L"",L"",L"",
										CDB_ControlProperties::eCVFUNC::ControlOnly,
										CDB_ControlProperties::CvCtrlType::eCvNU);

	rkEx.PurgeAll();
	CRank rkList;
	rkList.Add(pHS->GetCvTypeString(CDS_HmHubStation::eCVTAKV::eNone,false),CDS_HmHubStation::eCVTAKV::eNone,CDS_HmHubStation::eCVTAKV::eNone);
	rkList.Add(pHS->GetCvTypeString(CDS_HmHubStation::eCVTAKV::eKv,false),CDS_HmHubStation::eCVTAKV::eKv,CDS_HmHubStation::eCVTAKV::eKv);
	if (iCvTA) rkList.Add(pHS->GetCvTypeString(CDS_HmHubStation::eCVTAKV::eTAValve,false),CDS_HmHubStation::eCVTAKV::eTAValve,CDS_HmHubStation::eCVTAKV::eTAValve);
	rkList.Transfer(&m_ComboCv);

	// Find current selection
	for (int i=0; i<m_ComboCv.GetCount(); i++)
	{
		if (m_ComboCv.GetItemData(i) == pHS->GetCvTaKv())
		{
			m_ComboCv.SetCurSel(i);
			OnCbnSelchangeCombocv();
			break;
		}
	}
}

void CDlgHubCvCB::FillComboCtrlType()
{
	m_ComboCvCtrlType.ResetContent();

	if ((CDS_HmHubStation::eCVTAKV)m_ComboCv.GetItemData(m_ComboCv.GetCurSel())!=CDS_HmHubStation::eCVTAKV::eTAValve)
	{
		int i = 0;
		_string str = CDB_ControlProperties::GetCvCtrlTypeStr(CDB_ControlProperties::CvCtrlType::eCvProportional);
		i = m_ComboCvCtrlType.AddString(str.c_str());
		m_ComboCvCtrlType.SetItemData(i,CDB_ControlProperties::CvCtrlType::eCvProportional);
		str = CDB_ControlProperties::GetCvCtrlTypeStr(CDB_ControlProperties::CvCtrlType::eCvOnOff);
		i = m_ComboCvCtrlType.AddString(str.c_str());
		m_ComboCvCtrlType.SetItemData(i,CDB_ControlProperties::CvCtrlType::eCvOnOff);
		m_ComboCvCtrlType.SetCurSel(0);
		m_ComboCvCtrlType.EnableWindow(m_ComboCvCtrlType.GetCount() > 1);
		GetDlgItem(IDC_STATICCONTROLTYPE)->EnableWindow(m_ComboCvCtrlType.GetCount() > 1);
	}
	else
	{
		CRankEx RkList;
		int iCvProp = GetpTADB()->GetTaCVList(&RkList,CTADatabase::eForHMCv, true,
												CDB_ControlProperties::CV2W3W::CV2W,
												L"",L"",L"",L"",L"",L"",
												CDB_ControlProperties::eCVFUNC::ControlOnly,
												CDB_ControlProperties::CvCtrlType::eCvProportional);
		int i = -1;
		if (iCvProp)
		{
			_string str = CDB_ControlProperties::GetCvCtrlTypeStr(CDB_ControlProperties::CvCtrlType::eCvProportional);
			i = m_ComboCvCtrlType.AddString(str.c_str());
			m_ComboCvCtrlType.SetItemData(i,CDB_ControlProperties::CvCtrlType::eCvProportional);
		}
		int iCvOnOff = GetpTADB()->GetTaCVList(&RkList,CTADatabase::eForHMCv,true,
												CDB_ControlProperties::CV2W3W::CV2W,
												L"",L"",L"",L"",L"",L"",
												CDB_ControlProperties::eCVFUNC::ControlOnly,
												CDB_ControlProperties::CvCtrlType::eCvOnOff);
		if (iCvOnOff)
		{
			_string str = CDB_ControlProperties::GetCvCtrlTypeStr(CDB_ControlProperties::CvCtrlType::eCvOnOff);
			i = m_ComboCvCtrlType.AddString(str.c_str());
			m_ComboCvCtrlType.SetItemData(i,CDB_ControlProperties::CvCtrlType::eCvOnOff);
		}
		
		m_ComboCvCtrlType.SetCurSel(0);
		m_ComboCvCtrlType.EnableWindow( ( m_ComboCvCtrlType.GetCount() > 1) ? TRUE : FALSE );
		GetDlgItem(IDC_STATICCONTROLTYPE)->EnableWindow( ( m_ComboCvCtrlType.GetCount() > 1) ? TRUE : FALSE );

		OnCbnSelchangeComboctrltype();
	}
}

void CDlgHubCvCB::FillComboConnect()
{
	CRankEx rkList;
	m_ComboConnect.ResetContent();

	// Get current selected pipe
	CDB_Pipe *pPipe = (CDB_Pipe *)( m_pHM->GetpPipe( CDS_HydroMod::eHMObj::eCV )->GetIDPtr().MP );

	if( NULL == pPipe )
	{
		return; // No pipe selected
	}
	
	// Find DN field based on Pipe size, SizeBelow and SizeAbove from technical parameters
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );
	int iDNMax = iDN + m_pTechParam->GetSizeShiftAbove();
	int iDNMin = __max( iDN + m_pTechParam->GetSizeShiftBelow(), 0 );

	// Request available connection for 2Ways control valve, control Only
	m_pTADB->GetTaCVConnList( &rkList,CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::CV2W, L"", L"", L"",
				CDB_ControlProperties::eCVFUNC::ControlOnly, (CDB_ControlProperties::CvCtrlType)m_ComboCvCtrlType.GetItemData( m_ComboCvCtrlType.GetCurSel() ) );
	
	rkList.Transfer( &m_ComboConnect );

	if( m_ComboConnect.GetCount() > 0 )
	{
		m_ComboConnect.SetCurSel( 0 );
	}
	
	m_ComboConnect.EnableWindow( ( m_ComboConnect.GetCount() > 1) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICCONNECT )->EnableWindow( ( m_ComboConnect.GetCount() > 1) ? TRUE : FALSE );
	OnCbnSelchangeComboconnect();
}

void CDlgHubCvCB::FillComboVersion()
{
	CRankEx rkList;
	m_ComboVersion.ResetContent();

	// NO connection selected
	if( 0 == m_ComboConnect.GetCount() )
	{
		return;
	}
	
	LPARAM lParam = m_ComboConnect.GetItemData( m_ComboConnect.GetCurSel() );

	if( NULL == lParam )
	{
		return;
	}

	// Get current selected pipe.
	CDB_Pipe *pPipe = (CDB_Pipe *)( m_pHM->GetpPipe( CDS_HydroMod::eHMObj::eCV )->GetIDPtr().MP );
	
	if( NULL == pPipe )
	{
		return; // No pipe selected
	}
	
	// Find DN field based on Pipe size, SizeBelow and SizeAbove from technical parameters
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );
	int iDNMax = iDN + m_pTechParam->GetSizeShiftAbove();
	int iDNMin = __max( iDN + m_pTechParam->GetSizeShiftBelow(), 0 );

	m_pTADB->GetTaCVVersList( &rkList, CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::CV2W, L"",L"",L"", ((CDB_StringID *)lParam )->GetIDPtr().ID,
			CDB_ControlProperties::eCVFUNC::ControlOnly, (CDB_ControlProperties::CvCtrlType)m_ComboCvCtrlType.GetItemData( m_ComboCvCtrlType.GetCurSel() ) );

	rkList.Transfer( &m_ComboVersion );

	if( m_ComboVersion.GetCount() > 0 )
	{
		m_ComboVersion.SetCurSel( 0 );
	}
	
	m_ComboVersion.EnableWindow(m_ComboVersion.GetCount() > 1);
	GetDlgItem( IDC_STATICVERSION )->EnableWindow( ( m_ComboVersion.GetCount() > 1 ) ? TRUE : FALSE );
	OnCbnSelchangeComboversion();
}

void CDlgHubCvCB::FillComboPN()
{
	CRankEx rkList;
	m_ComboPN.ResetContent();

	// NO connection selected.
	if( 0 == m_ComboConnect.GetCount() )
	{
		return;
	}

	// NO version selected.
	if( 0 == m_ComboVersion.GetCount() )
	{
		return;
	}
	
	LPARAM lParam = NULL;
	
	// Get current selected pipe.
	CDB_Pipe *pPipe = (CDB_Pipe *)( m_pHM->GetpPipe( CDS_HydroMod::eHMObj::eCV )->GetIDPtr().MP );

	if( NULL == pPipe )
	{
		return; // No pipe selected
	}
	
	// Find DN field based on Pipe size, SizeBelow and SizeAbove from technical parameters
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );
	int iDNMax = iDN + m_pTechParam->GetSizeShiftAbove();
	int iDNMin = __max( iDN + m_pTechParam->GetSizeShiftBelow(), 0 );
	
	CDB_StringID *pConnect = (CDB_StringID *)m_ComboConnect.GetItemData( m_ComboConnect.GetCurSel() );
	CDB_StringID *pVersion = (CDB_StringID *)m_ComboVersion.GetItemData( m_ComboVersion.GetCurSel() );
	
	if( NULL == pConnect || NULL == pVersion )
	{
		return;
	}

	m_pTADB->GetTaCVPNList( &rkList,CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::CV2W, L"", L"", L"", pConnect->GetIDPtr().ID, pVersion->GetIDPtr().ID,
			CDB_ControlProperties::eCVFUNC::ControlOnly, (CDB_ControlProperties::CvCtrlType)m_ComboCvCtrlType.GetItemData( m_ComboCvCtrlType.GetCurSel() ) );
	
	rkList.Transfer( &m_ComboPN );

	if( m_ComboPN.GetCount() > 0 )
	{
		m_ComboPN.SetCurSel( 0 );
	}
	
	m_ComboPN.EnableWindow( m_ComboPN.GetCount() > 1 );
	GetDlgItem( IDC_STATICPN )->EnableWindow( ( m_ComboPN.GetCount() > 1 ) ? TRUE : FALSE );
	OnCbnSelchangeCombopn();
}

void CDlgHubCvCB::OnCbnSelchangeCombocv()
{
	CDS_HmHubStation::eCVTAKV CvTaKv = (CDS_HmHubStation::eCVTAKV) m_ComboCv.GetItemData(m_ComboCv.GetCurSel());
	// Reset following combo content
	m_ComboCvCtrlType.ResetContent();
	m_ComboConnect.ResetContent();
	m_ComboVersion.ResetContent();
	m_ComboPN.ResetContent();
	GetDlgItem(IDC_STATICCONTROLTYPE)->EnableWindow(false);
	GetDlgItem(IDC_STATICCONNECT)->EnableWindow(false);
	GetDlgItem(IDC_STATICVERSION)->EnableWindow(false);
	GetDlgItem(IDC_STATICPN)->EnableWindow(false);
	// Enable or disable following Combo
	m_ComboCvCtrlType.EnableWindow(false);
	m_ComboConnect.EnableWindow(false);
	m_ComboVersion.EnableWindow(false);
	m_ComboPN.EnableWindow(false);
	switch(CvTaKv)
	{
		case CDS_HmHubStation::eCVTAKV::eNone:
		break;
		case CDS_HmHubStation::eCVTAKV::eKv:
		case CDS_HmHubStation::eCVTAKV::eTAValve:
			m_ComboCvCtrlType.EnableWindow(true);
			GetDlgItem(IDC_STATICCONTROLTYPE)->EnableWindow(true);
			FillComboCtrlType();
		break;
	}
}

void CDlgHubCvCB::OnCbnSelchangeComboctrltype()
{
	if ((CDS_HmHubStation::eCVTAKV)m_ComboCv.GetItemData(m_ComboCv.GetCurSel())==CDS_HmHubStation::eCVTAKV::eTAValve)
	{
		m_ComboConnect.EnableWindow(true);
		m_ComboVersion.EnableWindow(true);
		m_ComboPN.EnableWindow(true);
		GetDlgItem(IDC_STATICCONNECT)->EnableWindow(true);
		GetDlgItem(IDC_STATICVERSION)->EnableWindow(true);
		GetDlgItem(IDC_STATICPN)->EnableWindow(true);
		FillComboConnect();
	}
}

void CDlgHubCvCB::OnCbnSelchangeComboconnect()
{
	FillComboVersion();
}

void CDlgHubCvCB::OnCbnSelchangeComboversion()
{
	FillComboPN();
}

void CDlgHubCvCB::OnCbnSelchangeCombopn()
{
}

void CDlgHubCvCB::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// Do not call CDlgSpreadCB::OnPaint() for painting messages
}
