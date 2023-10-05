//
// DialogKvCvInput.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "Utilities.h"
#include "Hydronic.h"

#include "DlgKvCvInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExtKvCvInput dialog


CDlgExtKvCvInput::CDlgExtKvCvInput(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgExtKvCvInput::IDD, pParent)
{
	m_dFlow = m_dRho = 0.0;
	m_pdKvCv = NULL;
}


void CDlgExtKvCvInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITKVCV, m_EditKvCv);
	DDX_Control(pDX, IDC_EDITFLOW, m_EditFlow);
	DDX_Control(pDX, IDC_EDITDP, m_EditDp);
}


BEGIN_MESSAGE_MAP(CDlgExtKvCvInput, CDialogEx)
	ON_EN_CHANGE(IDC_EDITKVCV, OnChangeEditkvcv)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExtKvCvInput member functions

int CDlgExtKvCvInput::Display( double *pdKvCv, double dFlow )
{
	m_dFlow = dFlow;
	m_pdKvCv = pdKvCv;

	// Get and store density of project fluid
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProject();
	ASSERT( NULL != pWC );

	m_dRho = pWC->GetDens();

	return DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgExtKvCvInput message handlers

BOOL CDlgExtKvCvInput::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGKVCVINPUT_STATICFLOW);
	GetDlgItem(IDC_STATICFLOW)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGKVCVINPUT_STATICDP);
	GetDlgItem(IDC_STATICDP)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGKVCVINPUT_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGKVCVINPUT_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);
	str.Empty();

	// Init Unit Pointer
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	TCHAR name[_MAXCHARS];

	// Retrieve User default for KvCv and initialize KvCv static and dialog caption
	CString tmpstr;
	if (!pUnitDB->GetDefaultUnitIndex(_C_KVCVCOEFF))
	{
		str=TASApp.LoadLocalizedString(IDS_DLGKVCVINPUT_KV);
		GetDlgItem(IDC_STATICKVCV)->SetWindowText(str);
		str=TASApp.LoadLocalizedString(IDS_KV);
	}
	else
	{
		str=TASApp.LoadLocalizedString(IDS_DLGKVCVINPUT_CV);
		GetDlgItem(IDC_STATICKVCV)->SetWindowText(str);
		str=TASApp.LoadLocalizedString(IDS_CV);
	};
	FormatString(tmpstr,IDS_DLGKVCVINPUT_CAPTION,str);
	SetWindowText(tmpstr);
	// Flow
	GetNameOf(pUnitDB->GetUnit(_U_FLOW,pUnitDB->GetDefaultUnitIndex(_U_FLOW)),name);
	GetDlgItem(IDC_STATICFLOWUNIT)->SetWindowText(name);
	// Dp
	GetNameOf(pUnitDB->GetUnit(_U_DIFFPRESS,pUnitDB->GetDefaultUnitIndex(_U_DIFFPRESS)),name);
	GetDlgItem(IDC_STATICDPUNIT)->SetWindowText(name);

	if (m_dFlow != 0.0)
		m_EditFlow.SetWindowText(WriteDouble(CDimValue::SItoCU(_U_FLOW,m_dFlow),3,0));
	if (*m_pdKvCv != 0.0)
		m_EditKvCv.SetWindowText(WriteDouble(CDimValue::SItoCU(_C_KVCVCOEFF,*m_pdKvCv),3,0));
						
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExtKvCvInput::OnOK() 
{
	CDialogEx::OnOK();
}

void CDlgExtKvCvInput::OnChangeEditkvcv() 
{
	CString str;
	m_EditKvCv.GetWindowText(str);
	switch (ReadDouble(str,m_pdKvCv))
	{
	case RD_EMPTY:
		m_EditDp.SetWindowText(_T(""));
		return;
	case RD_NOT_NUMBER:
		if (str == _T(".") || str == _T(",") || str.Right(1) == _T("e") || 
			str.Right(2) == _T("e+") || str.Right(2) == _T("e-")) 
			return;
		TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
		m_EditKvCv.SetSel(0, -1);
		m_EditKvCv.SetFocus();
		m_EditDp.SetWindowText(_T(""));
		return;
		break;
	case RD_OK:
		if (*m_pdKvCv < 0.0)
		{
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
			m_EditKvCv.SetSel(0, -1);
			m_EditKvCv.SetFocus();
			return;
		}
		else
		{
			*m_pdKvCv = CDimValue::CUtoSI(_C_KVCVCOEFF,*m_pdKvCv);
			if ((m_dFlow != 0.0)&&(*m_pdKvCv != 0.0))
			{
				double Dp;
				Dp = CalcDp(m_dFlow,*m_pdKvCv,m_dRho);
				m_EditDp.SetWindowText(WriteDouble(CDimValue::SItoCU(_U_DIFFPRESS,Dp),3,0));
			}
			else
				m_EditDp.SetWindowText(_T(""));
		};
		break;
	};
	return;
}
