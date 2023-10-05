// editmode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditModePropPage : Property page dialog

class CEditModePropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CEditModePropPage)
    DECLARE_OLECREATE_EX(CEditModePropPage)

// Constructors
public:
    CEditModePropPage();
  static GUID GetGuid() {return guid;};
// Dialog Data
    //{{AFX_DATA(CEditModePropPage)
    enum { IDD = IDD_EDIT_MODE };
    int     m_nEditEnterAction;
    BOOL    m_bArrowsExitEditMode;
    BOOL    m_bEditModePermanent;
    BOOL    m_bEditModeReplace;
    BOOL    m_bProcessTab;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CEditModePropPage)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

