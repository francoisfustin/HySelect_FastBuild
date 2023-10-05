#pragma once
#include "ModuleVersion.h"
#include "TASelect.h"
class CTenderDlg : public CDialogEx
{
public:
	CTenderDlg( CWnd *pParent = NULL );

	// Donn�es de bo�te de dialogue
	enum { IDD = IDD_DLGTENDER };

	void SetTender( const _string &artName, const _string &artNum, const _string &tenderText );

protected:
	virtual void DoDataExchange( CDataExchange *pDX );  // Prise en charge de DDX/DDV

	// Impl�mentation
protected:
	virtual BOOL OnInitDialog();
	_string m_artName;
	_string m_artNum;
	_string m_tender;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCopyclipboard();
};

