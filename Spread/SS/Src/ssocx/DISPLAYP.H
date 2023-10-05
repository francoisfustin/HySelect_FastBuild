// displayp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDisplayPropPage : Property page dialog

class CDisplayPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CDisplayPropPage)
    DECLARE_OLECREATE_EX(CDisplayPropPage)

// Constructors
public:
    CDisplayPropPage();
  static GUID GetGuid() {return guid;};
// Dialog Data
    //{{AFX_DATA(CDisplayPropPage)
	enum { IDD = IDD_DISPLAY };
    BOOL    m_bGridShowHoriz;
    BOOL    m_bGridShowVert;
    BOOL    m_bGridSolid;
    BOOL    m_bRedraw;
    BOOL    m_bScrollBarExt;
    BOOL    m_bScrollBarMaxAlign;
    BOOL    m_bScrollBarShowMax;
    int     m_nScrollBars;
    int     m_nButtonDrawMode;
	int		m_bkColorStyle;
	//}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CDisplayPropPage)
    afx_msg void OnSelendokScrollbars();
    virtual BOOL OnInitDialog();
    afx_msg void OnBdmCurrentCell();
    afx_msg void OnBdmCurrentCol();
    afx_msg void OnBdmCurrentRow();
    afx_msg void OnBdmAlways();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

