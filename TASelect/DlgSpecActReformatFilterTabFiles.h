#pragma once


#include "DialogExt.h"
#include "afxwin.h"
#include "ProductSelectionParameters.h"
#include "TUProductSelectionHelper.h"

class CDlgSpecActReformatFilterTabFiles : public CDialogExt
{
public:
	enum { IDD = IDD_DLGSPECACT_REFORMATFILTERTAB };

	CDlgSpecActReformatFilterTabFiles( CWnd *pParent = NULL );
	virtual ~CDlgSpecActReformatFilterTabFiles() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedOpenFilterTabPath();
	virtual afx_msg void OnBnClickedStartReformat();
	virtual afx_msg void OnBnClickedClear();

// Private methods.
private:
	void _FindFile( CString strPath, int &iDepth );
	void _ReformatTabFilterFile( CString strFilePath );
	bool _FormatString( std::vector<CString> &vecTokens, std::vector<int> &vecSeparatorPos, CString &strFormatted );

// Private variables.
private:
	CEdit m_EditFilterTabPath;
	CListBox m_List;
};
