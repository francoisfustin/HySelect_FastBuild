// opmodepr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpModePropPage : Property page dialog

class COpModePropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(COpModePropPage)
    DECLARE_OLECREATE_EX(COpModePropPage)

// Constructors
public:
    COpModePropPage();
  static GUID GetGuid() {return guid;};
// Dialog Data
    //{{AFX_DATA(COpModePropPage)
    enum { IDD = IDD_OP_MODE };
    BOOL    m_bAutoSize;
    BOOL    m_bNoBeep;
    BOOL    m_bNoBorder;
    int     m_nOperationMode;
    BOOL    m_bProtect;
    BOOL    m_bRetainSelBlock;
    int     m_nUnitType;
    int     m_nUserResize;
    int     m_nSelectBlockOptions;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(COpModePropPage)
    afx_msg void OnNormal();
    afx_msg void OnOpmodeExtend();
    afx_msg void OnOpmodeMulti();
    afx_msg void OnOpmodeReadonly();
    afx_msg void OnOpmodeRow();
    afx_msg void OnOpmodeSingle();
    afx_msg void OnSelblksCols();
    afx_msg void OnSelblksRows();
    afx_msg void OnSelblksBlks();
    afx_msg void OnSelblksAll();
    virtual BOOL OnInitDialog();
    afx_msg void OnUserResizeCols();
    afx_msg void OnUserResizeRows();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

