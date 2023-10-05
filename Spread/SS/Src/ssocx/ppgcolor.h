/////////////////////////////////////////////////////////////////////////////
// Structure used to store information about the system colors

struct SysColorsInfo
{
	UINT    nStrID;         // The id of the resource describing this system color
	int     nColor;         // The system color index
};

///////////////////////////////////////////////////////////////////////////////
// CMyColorButton: used by CMyColorPropPage

class CMyColorButton : public CButton
{
public:
	CMyColorButton(void);
	void SetFaceColor(COLORREF colFace);
	COLORREF colGetFaceColor(void);
	void SetState(BOOL fSelected);
	static UINT idClicked;
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
private:
	BOOL m_fSelected;
	COLORREF m_colFace;
};

/////////////////////////////////////////////////////////////////////////////
// CMyColorPropPage

#define NB_COLORS   (16)

class CMyColorPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CMyColorPropPage)
	DECLARE_OLECREATE_EX(CMyColorPropPage)

// Construction
public:
	CMyColorPropPage();   // Constructor
  static GUID GetGuid() {return guid;};

// Dialog Data
	//{{AFX_DATA(CColorPropPage)
	enum { IDD = AFX_IDD_PROPPAGE_COLOR };
	CComboBox   m_SysColors;
	CComboBox   m_ColorProp;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
	virtual BOOL OnInitDialog(void);
	virtual BOOL OnEditProperty(DISPID dispid);
	virtual void OnObjectsChanged();
    void FillPropnameList(REFGUID guid, int nIndirect, CComboBox& combo);
	void FillSysColors();
	BOOL SetColorProp(CDataExchange* pDX, COLORREF color, LPCTSTR pszPropName);
    BOOL GetColorProp(CDataExchange* pDX, COLORREF* pcolor, LPCTSTR pszPropName);
	BOOL GetColorProp(unsigned long* pcolor, LPCTSTR pszPropName);
	void OnSelchangePropname(CComboBox& combo);
	BOOL OnEditProperty(DISPID dispid, CComboBox& combo);

	CString m_strPropName;
	int m_iPropName;

private:
	CMyColorButton m_Buttons[NB_COLORS];
	CMyColorButton *m_pSelectedButton;

	void SetButton(CMyColorButton *Button);

	// Generated message map functions
	//{{AFX_MSG(CMyColorPropPage)
	afx_msg void OnSelchangeColorprop();
	afx_msg void OnSelect(void);
	afx_msg void OnSelchangeSystemcolors();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
