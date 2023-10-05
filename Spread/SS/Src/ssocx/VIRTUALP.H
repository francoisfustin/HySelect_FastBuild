// virtualp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVirtualPropPage : Property page dialog

class CVirtualPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CVirtualPropPage)
    DECLARE_OLECREATE_EX(CVirtualPropPage)

// Constructors
public:
    CVirtualPropPage();
  static GUID GetGuid() {return guid;};
// Dialog Data
    //{{AFX_DATA(CVirtualPropPage)
    enum { IDD = IDD_VIRTUAL_MODE };
    BOOL    m_bVirtualMode;
    long    m_lVMaxRows;
    long    m_lVOverlap;
    long    m_lVRows;
    BOOL    m_bVScrollBuffer;
    BOOL    m_bVScrollSpecial;
    int     m_nVScrollSpecialType;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CVirtualPropPage)
    afx_msg void OnVirtualMode();
    afx_msg void OnVscrollspec();
    afx_msg void OnVsstypeHomeend();
    afx_msg void OnVsstypeLineupdwn();
    afx_msg void OnVsstypePageupdwn();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

