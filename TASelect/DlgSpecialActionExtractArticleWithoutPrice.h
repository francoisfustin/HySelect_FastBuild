#pragma once


#include "DialogExt.h"
#include "afxwin.h"

class CDlgSpecialActionExtractArticleWithoutPrice : public CDialogExt
{
public:
	enum { IDD = IDD_DLGSPECACT_EXTRACTARTICLEWITHOUTPRICE };

	CDlgSpecialActionExtractArticleWithoutPrice( CWnd *pParent = NULL );
	virtual ~CDlgSpecialActionExtractArticleWithoutPrice() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedOpenFileToSave();
	virtual afx_msg void OnBnClickedExtract();

// Private variables.
private:
	CEdit m_EditFileToSave;
	CListBox m_List;
	CString m_strFileName;
};
