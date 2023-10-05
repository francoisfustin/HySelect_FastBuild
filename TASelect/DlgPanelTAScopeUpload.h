#pragma once


#include "wizard.h"
#include "TreeListCtrl.h"
#include "TCImageList.h"
#include "afxwin.h"

class CDlgPanelTAScopeUpload : public CDlgWizard
{
public:
	enum { IDD = IDD_PANEL_TASCOPE_UPLOAD };

	CDlgPanelTAScopeUpload( CWnd* pParent = NULL );
	virtual ~CDlgPanelTAScopeUpload() {}

protected:
	enum eTreeImg
	{
		etiNetwork = 0,
		etiNetworkS ,	// selected
		etiHMmodule ,
		etiHMmoduleS,	// selected
		etiHMTU,
		etiHMTUS,		// selected
		etiHUB ,
		etiHUBS,		// Selected
		etiOpenBox,
		etiClosedBox,
		etiOExclamation = 10,
		etiOCrossMark ,
		etiOMissingPV,
		etiOWhiteCross,
		etiOClosedBox
	};

	enum eTreeOverlay
	{
		etoExclamation = 1,
		etoCrossMark = 2,
		etoMissingPV = 3,
		etoWhiteCross = 4,
		etoClosedBox = 5,
	};
	
	enum eTreeCheckBox
	{
		etcbSquare,
		etcbSquareGray,
		etcbSquarePartial,
		etcbDot
	};

	typedef struct
	{
		bool bCheck;
		bool bBlackBox;
		bool bIncludedintoBB; 
	}m_UserData;

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnBnClickedPreview();
	afx_msg void OnNMClickTree( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnCbStateChanged( NMHDR* pNmHdr, LRESULT* pResult );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' virtual methods.
	virtual BOOL OnWizCancel();
	virtual BOOL OnWizNext();
	virtual BOOL OnWizFinish();
	virtual bool OnAfterActivate();
	virtual int OnWizButFinishTextID();
	virtual int OnWizButNextTextID();

	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AtLeastOneChildrenSelect( HTREEITEM hItem );
	void CheckAllChildren( HTREEITEM hItem, bool bFlag );
	void SetBlackBoxModule( HTREEITEM hItem, bool bFlag );

	bool FillTADSTreeNode( HTREEITEM hPItem, CTable* pTab );
	void FillTADSHNTree();
	bool FillTADSXTreeNode( HTREEITEM hPItem, CTable* pTab );
	void FillTADSXHNTree();

	bool Upload();

// Protected variables.
protected:
	CDlgWizTAScope		*m_pParent;
	CBrush				m_brWhiteBrush;
	CStatic				m_StaticVersionNbr;
	CTreeListCtrl		m_TreeTADS;
	CTreeListCtrl		m_TreeTADSX;
	CTADataStructX		m_TADSX;
	CTAMetaData			m_MetaData;
	CTCImageList		m_TreeImageList;
	// Don't use a CTCImageList for status, image appear shifted
	CImageList			m_TreeCheckBox;
	CMFCButton			m_ButtonPreview;
	CString				m_Pathfn;
	CString				m_PlantName;
	CDS_ProjectParams * m_pPrjParam;
	UINT_PTR			m_nTimer;
};
