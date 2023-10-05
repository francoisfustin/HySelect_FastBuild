// DlgQuickMeasNote.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "SSheetQuickMeas.h"
#include "DlgQuickMeasNote.h"
#include "afxdialogex.h"


// DlgQuickMeasNote dialog

IMPLEMENT_DYNAMIC(DlgQuickMeasNote, CDialogEx)

DlgQuickMeasNote::DlgQuickMeasNote(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLGQUICK_MEAS_NOTE, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

DlgQuickMeasNote::DlgQuickMeasNote( CString strOldNote, CWnd* pParent)
	: CDialogEx(IDD_DLGQUICK_MEAS_NOTE, pParent)
{
	if (0 < strOldNote.GetLength())
	{
		CString str = TASApp.LoadLocalizedString(IDS_SHEETQM_NOTE);
		str.Replace(_T("%1"), _T(""));
		strOldNote.Replace(str, _T(""));
		m_OldEditNoteValue = strOldNote;
	}
}

DlgQuickMeasNote::~DlgQuickMeasNote()
{
}


BEGIN_MESSAGE_MAP(DlgQuickMeasNote, CDialogEx)
	ON_BN_CLICKED(IDCANCELBUTTONNOTE, &DlgQuickMeasNote::OnBnClickedCancelbuttonnote)
	ON_BN_CLICKED(IDOKBUTTONNOTE, &DlgQuickMeasNote::OnBnClickedOkbuttonnote)
	ON_EN_CHANGE(IDC_EDITNOTE, &DlgQuickMeasNote::OnEnChangeEditnote)
END_MESSAGE_MAP()


void DlgQuickMeasNote::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITNOTE, m_EditNoteOnMeas);
	DDX_Control(pDX, IDOKBUTTONNOTE, m_ButtonOkNote);
}

BOOL DlgQuickMeasNote::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString strWin = TASApp.LoadLocalizedString(IDS_DLGQMNOTE_TITLE);	
	SetWindowTextW(strWin);
	if (0 != StringCompare(m_OldEditNoteValue, _T("")))
	{
		m_EditNoteOnMeas.SetWindowTextW(m_OldEditNoteValue);
	}

	return FALSE;
}

void DlgQuickMeasNote::OnBnClickedCancelbuttonnote()
{
	m_EditNoteOnMeas.SetWindowTextW(m_OldEditNoteValue);
	CDialogEx::OnCancel();
}

void DlgQuickMeasNote::OnBnClickedOkbuttonnote()
{
	m_EditNoteOnMeas.GetWindowTextW(m_NewEditNoteValue);
	CDialogEx::OnOK();
}


void DlgQuickMeasNote::OnEnChangeEditnote()
{
	int isizeText = m_EditNoteOnMeas.GetWindowTextLengthW();
	if (m_EditNoteOnMeas.GetModify())
	{
		m_ButtonOkNote.EnableWindow(TRUE);
	}
	if (0 >= isizeText)
	{
		if (0 == StringCompare(m_OldEditNoteValue, _T("")))
		{
			m_ButtonOkNote.EnableWindow(FALSE);
		}
	}
	m_EditNoteOnMeas.SetFocus();
	m_EditNoteOnMeas.SetSel(isizeText, isizeText, false);
}

CString DlgQuickMeasNote::GetTextNote()
{
	return m_NewEditNoteValue;
}