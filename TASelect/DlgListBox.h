//
// DialogListBox.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDlgListBox dialog

class CDlgListBox : public CDialogEx
{
// Construction
public:
	CDlgListBox(CWnd* pParent = NULL);   // standard constructor
	~CDlgListBox(){ClearAllLines();};	// standard destructor

// Dialog Data
	enum { IDD = IDD_DLGLISTBOX };
	CButton	m_ButtonIgnore;
	// HYS-1291 : Add checkbox to allow the lock of hydromod deleted product
	CButton	m_ButtonCheckDelProd;
	CStatic	m_Static2;
	CEdit	m_Edit2;
	CEdit	m_Edit1;
	CListCtrl	m_List;


// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
	
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangeListctrl(NMHDR* pNMHDR, LRESULT* pResult);	
	afx_msg void OnIgnore();
	afx_msg void OnBnClickedCheck();
	virtual afx_msg void OnOK();
	DECLARE_MESSAGE_MAP()


protected:
	struct s_StrItem
	{
		int		imgIndex;							// index of the associated image in the Image List
		CString str;								// String used if not empty
		int		ResID;								// if str is empty and ResID != -1 load string from Resource 
		int		colTxt1;							// if != -1 replace '%1' in loaded string with the text stored in colTxt1
		int		colTxt2;							// if != -1 replace '%2' in loaded string with the text stored in colTxt2
	};
	CList<s_StrItem,s_StrItem&> m_strList;

	struct s_OneLine
	{
		CArray<CString,CString&> str;				// array of string pointer
		LPARAM	lparam;								// optionnal LPARAM
		struct s_OneLine()
		{
			Reset();
		}
		void Reset()
		{
			str.RemoveAll();
			lparam = (LPARAM)0;
		}
	};

	struct	s_SortedItem 
	{
		int			subCol;
		CListCtrl	*pList;
	};

	CString m_strTitle;
	CString m_strEditStr1;
	CString m_strStaticStr;
	UINT m_uiImgListID;
	CImageList m_ListCtrlImageList;
	int		m_iCol;
	int		m_iSortCol;
	bool	m_bInitDialog;
	bool	m_fButIgnoreVisible;
	bool	m_bCustomColWidthList;
	int		m_ariListCustWidthCol[99];
	// HYS-1291 : value of checkbox m_ButtonCheckDelProd
	bool	m_bLockDeletedProd;
	// The checkbox is shown only when opening project with deleted HM product
	bool	m_bShowLockDeletedProd;

	CArray<CString,CString&>		m_strColHeader;	// Column header for CListCtrl
	s_OneLine						m_CurLine;		// one Line current line
	CArray<s_OneLine*,s_OneLine*>	m_strLines;		// Lines of CListCtrl

	void FillListCtrl();
	static int CALLBACK CompareStr(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	void SelectItem(int nItem);

public:
	// HYS-1291 : Add the possibility to show or not the checkbox
	int Display(int IDTitle, int IDEditStr1, int IDStatic2, int IDImgList, bool fButIgnoreVisible=false, bool fButCheckLockDeletedHM=false);
	int Display(CString Title, CString EditStr1, CString Static2, int IDImgList, bool fButIgnoreVisible=false);

	void UseCustCol(bool bUsed){m_bCustomColWidthList = bUsed;};
	bool IsCustWidthColUsed(){return m_bCustomColWidthList;};
	// The iPercTotalWidth value is express in % of the total listctrl width
	void SetColWidth(int icol, int iPercTotalWidth){m_ariListCustWidthCol[icol]=iPercTotalWidth;};

	POSITION AddStrToList(int imgIndex, CString str, int ResID=-1, int colTxt1 = -1,int colTxt2 = -1);
	POSITION AddStrToList(int imgIndex, int ResID, int colTxt1 = -1,int colTxt2 = -1);
	void SetColumn(int col);
	// Add a string under position 'col' into colum header
	void	AddHeaderStr (int col,CString str){ASSERT(col<m_iCol);m_strColHeader.SetAt(col,str);};
	void	AddHeaderStr (int col,int ResID){CString str;str=TASApp.LoadLocalizedString(ResID);AddHeaderStr(col,str);};
	
	// Clear Header line
	void	ClearHeader(){m_strColHeader.RemoveAll();};

	// Add a string under position 'col' into current line
	void	AddStr (int col,CString str){ASSERT(col<m_iCol);m_CurLine.str.SetAt(col,str);};
	// Clear Current line
	void	ClearLine(){m_CurLine.str.RemoveAll();m_CurLine.lparam = 0;};
	// Clear All lines
	void	ClearAllLines();
	// Add current line to the end of the array; grows the array if necessary 
	void	AddLine(LPARAM lparam);

	void Sort(int Col,bool fSort=true){ASSERT(Col<m_iCol);fSort?m_iSortCol = Col:m_iSortCol=-1;};
	
	// HYS-1291 : return the checkbox value
	bool IsLockHMDeletedProductEnable() { return m_bLockDeletedProd; }
};