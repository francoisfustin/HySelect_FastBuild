#pragma once

#include "HtmlViewer.h"
#include "afxwin.h"
// CDlgHtmlView dialog

class CDlgHtmlView : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgHtmlView)

public:
	CDlgHtmlView(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgHtmlView();

	void Display(CString url, int TitleIDS, int InfoIDS);

// Dialog Data
	enum { IDD = IDD_DLGHTMLVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CHtmlViewer *m_pHtmlView;
	CString m_strPageUrl;
	int m_iTitleIDS;
	int m_iInfoIDS;


	DECLARE_MESSAGE_MAP()
public:
	CStatic m_ImgInfo;
};
