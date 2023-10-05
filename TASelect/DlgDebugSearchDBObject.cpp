#include "stdafx.h"


#ifdef _DEBUG

#include "afxdialogex.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "DataBase.h"
#include "DlgDebugSearchDBObject.h"


IMPLEMENT_DYNAMIC( CDlgDebugSearchDBObject, CDialogEx )

CDlgDebugSearchDBObject::CDlgDebugSearchDBObject( CWnd *pParent )
	: CDialogEx(CDlgDebugSearchDBObject::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP( CDlgDebugSearchDBObject, CDialogEx )
	ON_EN_CHANGE( IDC_EDITSEARCHPATTERN, OnChangeEditSearchPattern )
	ON_BN_CLICKED(IDC_BUTTONSELECT, &CDlgDebugSearchDBObject::OnBnClickedButtonselect)
	ON_BN_CLICKED(IDC_BUTTONSEARCH, &CDlgDebugSearchDBObject::OnBnClickedButtonsearch)
END_MESSAGE_MAP()

void CDlgDebugSearchDBObject::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITSEARCHPATTERN, m_EditSearchPattern);
	DDX_Control(pDX, IDC_COMBORESULTS, m_ComboResults);
	DDX_Control(pDX, IDC_EDITSEARCHPATTERNBYAN, m_EditSearchByAn);
	DDX_Control(pDX, IDC_EDITIDLISTRESULT, m_EditIdListResult);
	DDX_Control(pDX, IDC_STATICRESULT, m_StaticNbFound);
}

BOOL CDlgDebugSearchDBObject::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_mapResults.clear();
	CDataList* pclDataList = TASApp.GetpTADB()->GetpDataList();

	IDPTR ObjectIDPtr = pclDataList->GetFirst();

	while( _NULL_IDPTR != ObjectIDPtr )
	{
		m_mapResults[CString( ObjectIDPtr.ID )] = false;
		ObjectIDPtr = pclDataList->GetNext();
	}

	return TRUE;
}

void CDlgDebugSearchDBObject::OnChangeEditSearchPattern()
{
	m_ComboResults.ResetContent();

	CString strID;
	m_EditSearchPattern.GetWindowText( strID );
	strID.Remove('\n');
	strID.Remove('\r');
	strID.Trim();

	int iLength = strID.GetLength();

	if( 0 == iLength )
	{
		return;
	}

	for( std::map<CString, bool>::iterator iter = m_mapResults.begin(); iter != m_mapResults.end(); iter++ )
	{
		if( -1 != iter->first.Find( strID ) )
		{
			m_ComboResults.AddString( (LPCTSTR)iter->first );
		}
	}

	m_ComboResults.SetCurSel( 0 );
}

void CDlgDebugSearchDBObject::OnBnClickedButtonselect()
{
	CString strID;
	m_ComboResults.GetLBText( m_ComboResults.GetCurSel(), strID );
	CData *pData = (CData *)( TASApp.GetpTADB()->Get( strID ).MP );
	__debugbreak();
}

void CDlgDebugSearchDBObject::OnBnClickedButtonsearch()
{
	CString strAN = _T("");
	CString strInformation = _T("");
	CArray <CString> arrStr;
	int iCount = 0;
	int iLine = -1;

	m_EditSearchByAn.GetWindowText(strAN);
	int iLength = strAN.GetLength();
	m_EditIdListResult.EnableWindow();
	m_EditIdListResult.SetSel(0, m_EditIdListResult.GetWindowTextLength());
	m_EditIdListResult.Clear();
	if (0 < iLength)
	{
		TASApp.GetpTADB()->SearchByArticleNumber(strAN, &arrStr);
		for (iCount = 0; iCount < arrStr.GetSize(); iCount++)
		{
			iLine = m_EditIdListResult.GetWindowTextLength();
			m_EditIdListResult.SetSel(iLine, iLine);
			m_EditIdListResult.ReplaceSel(arrStr.GetAt(iCount));
		}
		if (1 < iCount)
		{
			strInformation.Format(_T("%d IDs found"), arrStr.GetSize());
		}
		else if (1 == iCount)
		{
			strInformation.Format(_T("%d ID found"), arrStr.GetSize());
		}
		else
		{
			strInformation.Format(_T("ID not found with this pattern"));
		}
		m_StaticNbFound.SetWindowTextW(strInformation);
	}
	//__debugbreak();
}

#endif