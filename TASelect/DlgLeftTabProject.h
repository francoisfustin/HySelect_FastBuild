#pragma once


#include "DlgCreateModule.h"
#include "ProjTreeCtrl.h"
#include "MyToolTipCtrl.h"
#include "afxwin.h"
#include "SheetHMCalc.h"
#include "DlgProgBarExportXls.h"
#include "DialogExt.h"
#include "DlgLeftTabBase.h"

class CDS_HydroMod;
class CView;
class CF1Book;
class CSheetProj;
class CDlgLeftTabProject : public CDlgLeftTabBase, public CDlgCreateModule::INotification
{
public:
	enum { IDD = IDD_DLGLEFTTABPROJECT };

	enum OverlayMaskIndex
	{
		OMI_Undefined = 0,
		OMI_Exclamation = 1,
		OMI_CrossMark = 2,
		OMI_PartnerValve = 3,
		OMI_EditCircuit = 4,
		OMI_NewCircuit = 5
	};

	CDlgLeftTabProject( CWnd* pParent = NULL );
	virtual ~CDlgLeftTabProject();

	// Overrides 'CDlgLeftTabBase' public virtual methods.
	virtual void OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex );

	// Overrides 'CDlgLeftTabBase' public pure virtual methods.
	virtual void InitializeToolsDockablePaneContextID( void );
	virtual bool GetToolsDockablePaneContextID( int& iContextID );

	// Overrides 'CDlgCreateModule::INotification'.
	virtual void OnChangeLocatedIn( CDlgCreateModule* pDialog );

	// Allow to change display status of HydroMod in a cut mode.
	void SetFlagCut( std::vector<CDS_HydroMod*>* pvecHydromod );

	// Allow to reset all lines displayed in cut mode.
	void ResetFlagCut( void );

	// Allow to know if there at least one item in the tree that is in a cut mode.
	bool IsFlagCutSet( void );

	CMainFrame::RightViewList GetCurrentView() { return m_eCurrentRightView; }

	// Allow to position the selection on the root.
	void SelectRootItem( void );
	
	// Reset the tree and expand it to the item with a string matching 'strItemSelectedName'.
	void ResetTreeWithString( CString strItemSelectedName, bool bSelectItem = true, bool fResetCutFlag = true );

	// Reset Tree, expand it to the item with a stored data* matching 'pData'.
	void ResetTreeAndSelectHM( CDS_HydroMod* pHM, bool fResetCutFlag = true, bool fEditionOverlay = false );

	void ResetTree( bool fResetCutFlag = true );

	void ChangeSelection( CDS_HydroMod* pclHydroMod );

	void AddValve( int iPosition = -1 ) { _AddValve( iPosition ); }
	void AddModule( int iPosition = -1 ) { _AddModule( iPosition); }
	
	// Launch dialog box for editing pHM HydroMod properties.
	// Returns: 'true' if at least one setting has changed.
	bool EditHydroMod( CDS_HydroMod *pHM, int iDlgMode );

	// Return pointer on selected hydromod or NULL if it's not an hydromod.
	CDS_HydroMod* GetSelectedHM();

	// Call the correct right view redraw function.
	void RefreshRightView( void );
	
	void OnButtoncbi();
	void EnsureVisible(CDS_HydroMod* pHM, OverlayMaskIndex omi = OMI_Undefined);

// Public variables.
public:
	CButton			m_ButtonCbi;
	CProjTreeCtrl	m_Tree;

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnRclickTree( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnSelchangedTree( NMHDR* pNMHDR, LRESULT* pResult );

	afx_msg void OnFltMenuCreate();
	afx_msg void OnFltMenuAddValve();
	afx_msg void OnFltMenuFindIndexCircuit();
	afx_msg void OnFltMenuEdit();
	afx_msg void OnFltMenuRename();
	afx_msg void OnFltMenuDelete();
	afx_msg void OnFltMenuCut();
	afx_msg void OnFltMenuCopy();
	afx_msg void OnFltMenuPasteIn();
	afx_msg void OnFltMenuPasteBefore();
	afx_msg void OnFltMenuLock();
	afx_msg void OnFltMenuUnLock();
	afx_msg void OnFltMenuLockRecursive();
	afx_msg void OnFltMenuUnLockRecursive();
	afx_msg void OnFltMenuPrint();
	afx_msg void OnFltMenuPrintRecursive();
	afx_msg void OnFltMenuExport();
	afx_msg void OnFltMenuExportRecursive();
	afx_msg void OnFltMenuExportChart();
	afx_msg void OnFltMenuExportLDList();

// 	afx_msg void OnEditCut();	
// 	afx_msg void OnEditCopy();	
// 	afx_msg void OnEditPaste();
// 
// 	afx_msg void OnUpdateEditCut( CCmdUI* pCmdUI );
// 	afx_msg void OnUpdateEditCopy( CCmdUI* pCmdUI );
// 	afx_msg void OnUpdateEditPaste( CCmdUI* pCmdUI );
	afx_msg void OnUpdateMenuText( CCmdUI* pCmdUI );
	
	afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnProjectRefChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnResetTreeNeeded( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnResetTreeNeededCreate(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnResetTreeNeededEdit(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnTechParamChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );

// Protected variables.
protected:

	CMainFrame::RightViewList m_eCurrentRightView;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CImageList *m_pclTreeImageList;
	HTREEITEM m_hHNBranch;
    HTREEITEM m_hLoggedDataBranch;
    HTREEITEM 	m_hQuickMeasBranch;
    bool			m_fAtLeastOneHM;
	bool			m_fAtLeastOneLog;
	bool			m_fAtLeastOneQM;
	HTREEITEM		m_hItem;
	std::vector<HTREEITEM> m_vecSelectedItem;
	CBitmap			m_BitmapHmCalcUp;
	CBitmap			m_BitmapHmCalcDown;
	CBitmap			m_BitmapHmCalcDisabled;
	CBitmap			m_BitmapCBIConnect;
	std::map<UINT, CString>	m_mapUpdateMenuTooltip;
	std::map<int, int> m_mapContextIDList;

// Private methods.
private:
	// Reset Tree, expand it to the item with a UID matching 'uid'.
	void _ResetTree( unsigned int uid );

	// Recursive function that scan all the 'm_Tree' branches and set or reset OverlayMask based on  flow.
	void _SetOverlayMask( HTREEITEM hItem );
	
	// Create branch's name in function of his ancestor.
	void _ComposeModuleName( CString &str, HTREEITEM hItem, bool fCreate = false );
	
	// Recursive function that scan all CDS_HydoMod branches and add it into the tree.
	void _AddTreeBranch( IDPTR IDPtr, HTREEITEM hParentItem );

	// Find a tree item with the text matching str.
	HTREEITEM _FindhItem( CString &str, HTREEITEM hItem );
	
	// Find a tree item with the CData * matching.
	HTREEITEM _FindhItem( void *pData, HTREEITEM hItem );
	
	// Find a tree item with the matching CDS_HydroMod *.
	HTREEITEM _FindhItem( CDS_HydroMod *pHM, HTREEITEM hItem );

	void _AddModule( int iPosition = -1 );
	void _AddValve( int iPosition = -1 );

	void _DeleteChart( HTREEITEM hItem );
	CString _DeleteModule( HTREEITEM hItem );

	void _AddChildrenInCutList( CDS_HydroMod* pclParentHM );

	typedef enum PasteMode
	{
		PasteIn,
		PasteBefore
	};
	void _TabProjPaste( PasteMode ePasteMode );

};

extern CDlgLeftTabProject *pDlgLeftTabProject;
