// ssocxppg.h : Declaration of the CSsocxPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CSsocxPropPage : See ssocxppg.cpp for implementation.

class CSsocxPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CSsocxPropPage)
    DECLARE_OLECREATE_EX(CSsocxPropPage)

// Constructor
public:
  CSsocxPropPage();
  static GUID GetGuid() {return guid;};

// Dialog Data
    //{{AFX_DATA(CSsocxPropPage)
    enum { IDD = IDD_GENERAL };
    BOOL    m_bAllowCellOverflow;
    BOOL    m_bAllowDragDrop;
    BOOL    m_bAllowUserFormulas;
    BOOL    m_bAllowMultiBlocks;
    BOOL    m_bAutoCalc;
    BOOL    m_bAutoClipboard;
    BOOL    m_bFormulaSync;
    BOOL    m_bMoveActiveOnFocus;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CSsocxPropPage)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};
