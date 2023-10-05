// headersp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHeadersPropPage : Property page dialog

class CHeadersPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CHeadersPropPage)
    DECLARE_OLECREATE_EX(CHeadersPropPage)

// Constructors
public:
    CHeadersPropPage();
  static GUID GetGuid() {return guid;};
// Dialog Data
    //{{AFX_DATA(CHeadersPropPage)
    enum { IDD = IDD_HEADERS };
    long    m_lFrozenCols;
    long    m_lFrozenRows;
    long    m_lMaxCols;
    long    m_lMaxRows;
    BOOL    m_bRestrictCols;
    BOOL    m_bRestrictRows;
    int     m_nColHeaderDisplay;
    int     m_nRowHeaderDisplay;
    long    m_lStartingCol;
    long    m_lStartingRow;
    long    m_lVisibleCols;
    long    m_lVisibleRows;
    BOOL    m_bDisplayColHeaders;
    BOOL    m_bDisplayRowHeaders;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CHeadersPropPage)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

