#pragma once


#include "DlgLeftTabBase.h"
class CDlgLeftTabInfo : public CDlgLeftTabBase
{
public:
	CDlgLeftTabInfo( CWnd *pParent = NULL );
	virtual ~CDlgLeftTabInfo();

	// Dialog Data
	enum { IDD = IDD_DLGLEFTTABINFO };
	
// Public methods.
public:
	void ResetTree();
/**
 * This function creates the node "Untranslated catalogs" and inserts  
 * untranslated catalogs under.
 * @author awa
 * @param  (I) mainTree: The main CTreeCtrl on which documents are insert
 * @param  (IO) hRoot: the root node for mainTree
 * @param  (I) strDocToInsert: The name of the catalogs to insert
 * @param  (I) iItemIndex: The position of the catalog in docdb.csv
 * @remarks : Created for HYS-1057
*/
	void InsertUntranslated( CTreeCtrl *pMainTree, HTREEITEM *phRoot, CString strDocToInsert, int iItemIndex );

	// Overrides 'CDlgLTtabctrl::INotificationHandler'.
	virtual void OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex );

	// Overrides 'CDlgLeftTabBase' public pure virtual methods.
	virtual void InitializeToolsDockablePaneContextID( void );
	virtual bool GetToolsDockablePaneContextID( int &iContextID );
	
// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnItemExpandedTree( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnSelChangedTree( NMHDR *pNMHDR, LRESULT *pResult );	

	// User ask information about a product, lParam receive a pointer on ProductID.
	LRESULT OnDisplayDocumentation( WPARAM wParam, LPARAM lParam );

// Private methods.
private:
	void _CompleteDocInformation( CDB_Doc *pDoc );
	CDB_Doc *_FindCatFile( CDB_Doc *pDoc );
	bool _FindObjectInTree( CDB_Doc *pDoc, HTREEITEM *phParent );
	void _InsertDocs( std::map<int, CData *> *pMap, HTREEITEM *phParent );
	void _RemoveEmptyFolders( HTREEITEM *phParent );

public:
	void _CollapseAll( HTREEITEM hItem );

// Public variables.
public:
	CTreeCtrl m_Tree;

// Protected variables.
protected:
	CTADatabase *m_pTADB;
	bool m_bExcludeUnlinkedQrs;
	CImageList m_TreeImageList;
	CString m_strDocLanguage;			// Contains document language defined by user or current TASelect language.
	CString m_strVersionLang;			// Contains first language defined in "VERSION_TAB" in localdb.txt.
	CString m_strDocDBPath;
	// HYS-1057: boolean to avoid to create again the node "Untranslated catalogs"
	bool m_bUntranslatedDocExists;
	// HYS-1057: To keep the node "Untranslated catalogs"
	HTREEITEM hUnTranslatedRoot;
};

extern CDlgLeftTabInfo *pDlgLeftTabInfo;
