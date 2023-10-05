#pragma once


#include "afxwin.h"
#include "afxcmn.h"
#include "MyToolTipCtrl.h"
#include "utilities.h"
#include "ExtComboBox.h"
#include "EnBitmap.h"
#include "XGroupBox.h"
#include "FormViewEx.h"

#define DLGPRINT_RIGHTCLICK			0x80
#define DLGPRINT_TREEEXCLUSIVEMODE	DLGPRINT_RIGHTCLICK

class CDlgPrint : public CDialogEx, public IFormViewPrintNotification
{
	DECLARE_DYNAMIC( CDlgPrint )

public:
	enum { IDD = IDD_DLGPRINT };

	CDlgPrint( CWnd* pParent = NULL );
	virtual ~CDlgPrint();

	void Print_PrintPrev( bool fPreview );

	// Overrides 'IFormViewPrintNotification' methods (comes from 'CFormViewEx.h').
	virtual BOOL IFVPN_OnPreparePrinting( CPrintInfo* pInfo );
	virtual void IFVPN_OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void IFVPN_OnPrint( CDC* pDC, CPrintInfo* pInfo );
	virtual void IFVPN_OnEndPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void IFVPN_OnEndPrintPreview( CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView );
	virtual void IFVPN_OnFilePrint();
	
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );
	afx_msg void OnMove( int x, int y );

	afx_msg void OnBnClickedEditHeaderFooter();
	afx_msg void OnBnClickedEditProjectInfos();
	afx_msg void OnBnClickedPrintSetup();
	afx_msg void OnCheckClickedPrintProjectInfo();
	afx_msg void OnRadioClickedCondensedProjectStructure();
	afx_msg void OnRadioClickedProjectStructure();
	afx_msg void OnRadioClickedCircuitTables();
	afx_msg void OnRadioClickedDetailsCircuitView();
	afx_msg void OnBnClickedExtend();
	afx_msg void OnBnClickedCollapse();
	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedUnselect();
	afx_msg void OnCbnSelChangePrintingOutput();
	afx_msg void OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnNMRclickTree( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnBnClickedPreview();	
	afx_msg void OnBnClickedPrint();
	afx_msg void OnBnClickedCancel();

	// User click on the tree.
	// Display information according to the tree state.
	LRESULT OnCheckStateChange( WPARAM wParam, LPARAM lParam );
	
	// Initialize the combo list and return ID of the selected item.
	int InitCB( int iSelItemID = 0 );

	// Initialize the tree view in function of the selected item in combo.
	void InitTreeView( void );

	// Enable or disable the tree view and associated buttons.
	void EnableTreeView( bool fEnable );
	
	// Enable or disable group radio button project.
	void EnableGroupPrj( bool fEnable );

	// Recursive function to fill the tree view with Hydromod.
	void FillHMTreeView( CTable * pTab, HTREEITEM hParentItem, bool fOnlyModule );

	// Expand or collapse recursively all the tree.
	int TreeExpand( HTREEITEM hItem, UINT uiCode );

	// Select or unselect the 'hItem'.
	int TreeSelect( HTREEITEM hItem, BOOL bCheck, bool fSibling = false, bool fChildren = false );

	// Fill the tree view with LogData.
	void FillLDTreeView( void );
	
	// Fill the tree view with selected product category.
	void FillSPTreeView( void );
	
	// Fill the tree view with quick measurements products.
	void FillQMTreeView( void );

	// Fill a rank list with lParam of checked items; sort key is filled with position in the tree view.
	void FillCheckedItemRankLst( HTREEITEM hItem, double& dKey );

	// Verify is at least one hItem is checked.
	bool IsSomethingSelected( HTREEITEM hItem );
	
	// Enable or disable print and print preview button in function of selected item.
	void CheckPrintButtonState( HTREEITEM hItemRoot = NULL );

	void DrawPicture();

	bool PrintPreview();

// Public variables.
public:
	CEnBitmap m_Bmp;
	CStatic m_Picture;

	CExtNumEditComboBox m_CBList;
	CTreeCtrl m_Tree;
	CMFCButton m_ButExpand;
	CMFCButton m_ButCollapse;
	CMFCButton m_ButSelect;
	CMFCButton m_ButUnselect;
	CMFCButton m_ButPrintSetup;
	CMFCButton m_ButEditHeaderFooter;
	CMFCButton m_ButEditPrjInfos;
	
	CButton m_CheckPrintPrjInfo;
	CButton m_ButPrint;
	CButton m_ButPreview;

	CButton m_RadCondensedProjectStructure;
	CButton m_RadProjectStructure;
	CButton m_RadCircuitTables;
	CButton m_RadDetailedCircuitView;
	CXGroupBox m_GroupCircuit;
	
	CTADatastruct *m_pTADS;
	CImageList *m_pclTreeImageList;
	short m_TreeItemHeight;
	CMyToolTipCtrl m_ToolTip;
	CRScrollView *m_pRSV;
	CRank m_CheckedList;
	bool m_bMainModule;

// Private variables.
private:
	enum eCBItem
	{
		ecbSelProd   = IDS_DLGPRINT_SELPROD,
		ecbProject   = IDS_DLGPRINT_PROJECT,		
		ecbLogData   = IDS_DLGPRINT_LOGDATA,
		ecbQuickMeas = IDS_DLGPRINT_QUICKMEAS,
	};
	enum eLogData
	{
		eldSummary,
		eldChart,
		eldData
	};
	typedef struct sLogData
	{
		eLogData type;
		LPARAM   lparam;
	};
	typedef struct sTab
	{
		int iIDS;
		int iImages;
		IDPTR idptr;
	};

	CArray <sLogData> m_aLD;
	CArray <sTab> m_aTab;
	bool m_bInitialized;
	std::map<int, int> m_mapImageIndex;
};
