// DialogListBox.cpp : implementation file
//

#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"

#include "DlgListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgListBox dialog


CDlgListBox::CDlgListBox(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgListBox::IDD, pParent)
{
	m_strList.RemoveAll();
	m_strTitle = _T("");
	m_strEditStr1 = _T("");
	m_strStaticStr = _T("");
	m_uiImgListID = 0;
	m_iCol = 0;
	m_iSortCol = -1;
	m_bInitDialog = true;
	m_fButIgnoreVisible = false;
	m_bCustomColWidthList = false;
	ZeroMemory( &m_ariListCustWidthCol, sizeof( m_ariListCustWidthCol ) );
	m_strColHeader.RemoveAll();
	m_CurLine.Reset();
	m_strLines.RemoveAll();
	m_bLockDeletedProd = false;
	m_bShowLockDeletedProd = false;
}

void CDlgListBox::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgListBox)
	DDX_Control(pDX, IDC_IGNORE, m_ButtonIgnore);
	DDX_Control(pDX, IDC_STATIC2, m_Static2);
	DDX_Control(pDX, IDC_EDIT2, m_Edit2);
	DDX_Control(pDX, IDC_EDIT1, m_Edit1);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_CHECKDELPROD, m_ButtonCheckDelProd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgListBox, CDialogEx)
	//{{AFX_MSG_MAP(CDlgListBox)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChangeListctrl)
	ON_BN_CLICKED(IDC_IGNORE, OnIgnore)
	ON_BN_CLICKED(IDC_CHECKDELPROD, OnBnClickedCheck)
	ON_BN_CLICKED( IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgListBox message handlers

BOOL CDlgListBox::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	CString str;
	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_IGNORE);
	GetDlgItem(IDC_IGNORE)->SetWindowText(str);
	m_fButIgnoreVisible ? m_ButtonIgnore.ShowWindow(SW_SHOW) : m_ButtonIgnore.ShowWindow(SW_HIDE);

	// HYS-1291 : Initialize checkbox
	str=TASApp.LoadLocalizedString(IDS_LOCKDELPROD);
	m_ButtonCheckDelProd.SetWindowText(str);
	// m_bShowLockDeletedProd value comes from display call
	if( true == m_bShowLockDeletedProd )
	{
		m_ButtonCheckDelProd.ShowWindow(SW_SHOW);
	}
	else
	{
		m_ButtonCheckDelProd.ShowWindow( SW_HIDE );
	}
	// Build and attach an image list to m_List
	if (!m_ListCtrlImageList.GetSafeHandle())
		m_ListCtrlImageList.Create(m_uiImgListID,16,1,RGB(0,128,128));
	m_ListCtrlImageList.SetBkColor(CLR_NONE);
	m_List.SetImageList(&m_ListCtrlImageList,LVSIL_SMALL);

	// Get a full row selected
	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

	SetWindowText(m_strTitle);
	m_Edit1.SetWindowText(m_strEditStr1);
	m_Static2.SetWindowText(m_strStaticStr);
	m_List.DeleteAllItems();
	FillListCtrl();
	SelectItem(0);
	m_bInitDialog = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CDlgListBox::Display(int IDTitle, int IDEditStr1, int IDStatic2, int IDImgList, bool fButIgnoreVisible /*=false*/, bool fButCheckLockDeletedHM /*=false*/ )
{
	CString str1,str2,str3,str4;
	str1=TASApp.LoadLocalizedString(IDTitle);
	str2=TASApp.LoadLocalizedString(IDEditStr1);
	str3=TASApp.LoadLocalizedString(IDStatic2); 
	m_bShowLockDeletedProd = fButCheckLockDeletedHM;
	return Display(str1,str2,str3,IDImgList,fButIgnoreVisible);
}

int CDlgListBox::Display(CString Title, CString EditStr1, CString Static2, int IDImgList, bool fButIgnoreVisible /*=false*/)
{
	m_strTitle = Title;
	m_strEditStr1 = EditStr1;
	m_strStaticStr = Static2;
	m_uiImgListID = IDImgList;
	m_fButIgnoreVisible = fButIgnoreVisible;
	return DoModal();
}

void CDlgListBox::FillListCtrl()
{
	// Fill column headers, add columns 
	// Size of columns is defined by the size of columns header, 
	// it must be formatted with a sufficient number of space.  
	CString	str;
	CRect rect;
	m_List.GetWindowRect(&rect);
	int i;
	for (i=0; i<m_iCol; i++)
	{
		str=m_strColHeader.GetAt(i);
		if(IsCustWidthColUsed())
			m_List.InsertColumn(i,str,LVCFMT_LEFT, rect.Width()*m_ariListCustWidthCol[i]/100, i);
		else
		{
			CRect Clientrect;
			m_List.GetClientRect((LPRECT)&Clientrect);			
			m_List.InsertColumn(i,str,LVCFMT_LEFT, (int)(Clientrect.Width()/m_iCol), i);
		}
	}
	
	// Fill ListCtrl Lines with string lines from m_strLines
	for (i=m_strLines.GetSize()-1; i>=0; i--)	
	{
		s_OneLine	*pOneLine = m_strLines.GetAt(i);
		ASSERT (pOneLine);
		int item = m_List.InsertItem(0,pOneLine->str.GetAt(0));
		for (int j=1; j<pOneLine->str.GetSize(); j++)
			m_List.SetItem(item,j,LVIF_TEXT,pOneLine->str.GetAt(j),0,0,0,0);
		m_List.SetItem(item,0,LVIF_PARAM,_T(""),0,0,0,pOneLine->lparam);

		POSITION pos = (POSITION)m_List.GetItemData(item);
		s_StrItem strItem;
		strItem = m_strList.GetAt(pos);
		m_List.SetItem(item,0,LVIF_IMAGE,_T(""),strItem.imgIndex,0,0,NULL);
	}

	if (m_iSortCol >= 0 && m_iSortCol<m_iCol)
	{
		s_SortedItem SItem;
		SItem.pList = &m_List;
		SItem.subCol = m_iSortCol;
		m_List.SortItems(CompareStr,(LPARAM)&SItem);
	}	
}

POSITION CDlgListBox::AddStrToList(int imgIndex, CString str, int ResID/*=-1*/, int colTxt1/*=-1*/,int colTxt2/*=-1*/)
{
	s_StrItem strItem;
	strItem.imgIndex = imgIndex;
	strItem.str = str;
	strItem.ResID = ResID;
	strItem.colTxt1 = colTxt1;
	strItem.colTxt2 = colTxt2;
	return(m_strList.AddHead(strItem));
}

POSITION CDlgListBox::AddStrToList(int imgIndex, int ResID, int colTxt1/*=-1*/,int colTxt2/*=-1*/)
{
	s_StrItem strItem;
	strItem.imgIndex = imgIndex;
	strItem.str = _T("");
	strItem.ResID = ResID;
	strItem.colTxt1 = colTxt1;
	strItem.colTxt2 = colTxt2;
	return(m_strList.AddHead(strItem));
}

void CDlgListBox::SetColumn(int col)
{
	m_iCol = col;
	m_strColHeader.RemoveAll();		// Column header for CListCtrl
	m_CurLine.str.RemoveAll();		// one Line current line
	m_CurLine.lparam=0;
	m_strLines.RemoveAll();			// Lines of CListCtrl

	m_strColHeader.SetSize(m_iCol);
	m_CurLine.str.SetSize(m_iCol);

}

// Add current line to the end of the array; grows the array if necessary 
void CDlgListBox::AddLine(LPARAM lparam)
{
	m_CurLine.lparam=lparam;
	s_OneLine	*pOL = new (s_OneLine);
	ASSERT(pOL);
	pOL->lparam = lparam;
	pOL->str.Append(m_CurLine.str);
	m_strLines.Add(pOL);
	/*m_strLines.Add(m_CurLine);*/
}

void CDlgListBox::SelectItem(int nItem) 
{
	if (!m_List.GetItemCount()) return;
	m_List.SetItem(nItem, 0, LVIF_STATE, NULL, 0, LVIS_SELECTED, LVIS_SELECTED, 0);
	POSITION pos = (POSITION)m_List.GetItemData(nItem);
	if (pos)
	{
		CString str,rStr;
		const int BufSize = 50;
		TCHAR buf[BufSize];
		
		s_StrItem strItem;
		strItem = m_strList.GetAt(pos);
		if (strItem.str.IsEmpty())
		{
			if (strItem.ResID>0)
				str=TASApp.LoadLocalizedString(strItem.ResID);
		}
		else str = strItem.str;
		rStr=str;
		if (strItem.colTxt1>0)
		{
			m_List.GetItemText(nItem,strItem.colTxt1, buf, BufSize-1);
			FormatString(rStr,str,buf);
		}
		str=rStr;
		if (strItem.colTxt2>0)
		{
			m_List.GetItemText(nItem,strItem.colTxt2, buf, BufSize-1);
			FormatString(rStr,str,_T(""),buf);
		}
		m_Edit2.SetWindowText(rStr);
	}
	else
		m_Edit2.SetWindowText(_T(""));
}

void CDlgListBox::OnItemChangeListctrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if (m_bInitDialog) return;

	// If the user changes the selected item, display text associated with this Item
    if(pNMListView->iItem>=0 && (pNMListView->uNewState & LVIS_SELECTED))
    {
           SelectItem(pNMListView->iItem);
           TRACE(_T("\nItem:%d:%s"),pNMListView->iItem,(LPCTSTR)m_List.GetItemText(pNMListView->iItem,0));
    }
	else 
		m_Edit2.SetWindowText(_T(""));
	*pResult = 0;
}


void CDlgListBox::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Retrieve where the user clicked
/*	LVHITTESTINFO	ht={0};
	DWORD dwpos = GetMessagePos();
	ht.pt.x= GET_X_LPARAM(dwpos);
	ht.pt.y= GET_Y_LPARAM(dwpos);

	::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom,&ht.pt,1);

	// If the user clicks, display text associated with this Item
	int nItem = m_List.HitTest(&ht);
	if (nItem<0)	// Item not found
		nItem=m_List.SubItemHitTest(&ht);
	if (ht.iItem>=0)
	{
		CRect Rect,LabelRect;
		m_List.GetItemRect( ht.iItem, &Rect, LVIR_BOUNDS);
		if (Rect.PtInRect(ht.pt)) SelectItem(nItem);
	}
	else
		m_Edit2.SetWindowText(_T(""));*/
	
	*pResult = 0;
}

void CDlgListBox::ClearAllLines()
{
	s_OneLine	*pOneLine;
	
	for (int i=0;i<m_strLines.GetSize(); i++)
	{
		pOneLine = m_strLines.GetAt(i);
		pOneLine->str.RemoveAll();
		delete (pOneLine);
	}
} 

int CALLBACK CDlgListBox::CompareStr(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	s_SortedItem *SItem = (s_SortedItem *)lParamSort;
	// lParam1 & lParam2 are in fact the lParam value of each CStringList Item, 
	// !!! In "DlgCannotSend" case lParam value is used to store the info string ptr
	// so we can retrieve the current sorted Items.
	// Workaround: we do sorting test on those dataptr.
	if (lParam1<lParam2) return -1;
	if (lParam1>lParam2) return 1;
	else return 0;
}

void CDlgListBox::OnIgnore() 
{
	EndDialog(IDIGNORE); // This value is returned by DoModal!
}

void CDlgListBox::OnBnClickedCheck()
{
	m_bLockDeletedProd = ( BST_CHECKED == m_ButtonCheckDelProd.GetCheck() ) ? true : false;
}

void CDlgListBox::OnOK()
{
	CDialogEx::OnOK();
}
