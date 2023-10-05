#include "stdafx.h"
#include "mainfrm.h"
#include "TASelect.h"
#include "utilities.h"
#include "ModuleVersion.h"
#include "DlgHtmlView.h"

IMPLEMENT_DYNAMIC( CDlgHtmlView, CDialogEx )

CDlgHtmlView::CDlgHtmlView( CWnd* pParent )
	: CDialogEx( CDlgHtmlView::IDD, pParent )
{
	m_pHtmlView = new CHtmlViewer;					ASSERT( NULL != m_pHtmlView );
	m_strPageUrl = _T("");
	m_iTitleIDS = 0;
	m_iInfoIDS = 0;
}

CDlgHtmlView::~CDlgHtmlView()
{
}

void CDlgHtmlView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BMPINFO, m_ImgInfo);
}

void CDlgHtmlView::Display(CString url, int TitleIDS, int InfoIDS)
{
	m_strPageUrl = url;
	m_iTitleIDS = TitleIDS;
	m_iInfoIDS = InfoIDS;
	DoModal();
}

BEGIN_MESSAGE_MAP(CDlgHtmlView, CDialogEx)
END_MESSAGE_MAP()


// CDlgHtmlView message handlers
BOOL CDlgHtmlView::OnInitDialog() 
{
	if( NULL == m_pHtmlView )
		return FALSE;

	CDialogEx::OnInitDialog();
	// Initialize non-dynamic dialog strings
	CString str = TASApp.LoadLocalizedString(m_iTitleIDS);
	SetWindowText(str);
	if (m_iInfoIDS)
	{
		str = TASApp.LoadLocalizedString(m_iInfoIDS);
		CImageList* pclImgStripHome = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_StripHome );		ASSERT ( NULL != pclImgStripHome );
		if( NULL != pclImgStripHome )
			m_ImgInfo.SetIcon( (HICON)pclImgStripHome->ExtractIcon( CRCImageManager::ILSH_PanelInfo ) );
		GetDlgItem(IDC_EDITINFO)->SetWindowText(str);
		GetDlgItem(IDC_BMPINFO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDITINFO)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_BMPINFO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDITINFO)->ShowWindow(SW_HIDE);
	}
	CRect		sRect;
	CPoint		sPoint(0,0);
	GetDlgItem(IDC_STATICFRAME)->GetWindowRect(&sRect);
	ClientToScreen(&sPoint);
	sRect.left	 -= sPoint.x;
	sRect.right	 -= sPoint.x;
	sRect.top	 -= sPoint.y;
	sRect.bottom -= sPoint.y;

	m_pHtmlView->Create(NULL, NULL,WS_VISIBLE | WS_CHILD & (!WS_BORDER) /*AFX_WS_DEFAULT_VIEW*/,sRect, this,AFX_IDW_PANE_FIRST+100);
	m_pHtmlView->DisplayPage(m_strPageUrl);

	return true;
}

