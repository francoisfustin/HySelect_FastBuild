#pragma	once


class CDS_HydroMod;	
class CProjTreeCtrl	: public CTreeCtrl
{
public:	
	CProjTreeCtrl();
	virtual	~CProjTreeCtrl();

	// For each tree node we keep the associated view.
	struct STreeNode
	{
		CMainFrame::RightViewList eRightView;
		void *pObject;
	};
	CList<STreeNode,STreeNode&>	m_List;	

	// Return the view assigned	to a specific tree item.
	CMainFrame::RightViewList GetRightViewAt( HTREEITEM hItem );
	
	// Return a	void * (must be	a CDS_HydroMod*, CDS_LoggedData*, ...)	assigned to a specific tree item.
	void* GetObjectAt( HTREEITEM hITem );
	
	// Add a node in 'm_List', must be done each time we add something in the tree.
	POSITION AddToNodeList( CMainFrame::RightViewList eRightView, void *pObject = NULL );

	void SetItemCut( HTREEITEM hTreeItem );
	// If 'hTreeItem' is NULL then reset all.
	void ResetItemCut( HTREEITEM hTreeItem );
	bool IsItemCut( HTREEITEM hTreeItem );
	bool IsAtLeastOneItemCut( void );

// Protected methods.
protected:	
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
	afx_msg void OnPaint();
	afx_msg void OnBeginDrag( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnTimer( UINT_PTR nIDEvent );

	HTREEITEM HighlightDropTarget( CPoint point );
	BOOL IsItemExpanded( HTREEITEM hItem );
	void CopyChildren( HTREEITEM hDest, HTREEITEM hSrc );
	void CopyTree( HTREEITEM hDest, HTREEITEM hSrc, bool fNewItem = true );
	void MoveTree( HTREEITEM hDest, HTREEITEM hSrc, bool fNewItem = true );
	BOOL IsChildOf( HTREEITEM hItem1, HTREEITEM hItem2 );

// Protected variables.
protected:
	HTREEITEM m_hDragItem;
	CImageList* m_pImageList;
	BOOL m_fDragging;
	int m_iDelayInterval;
	int m_iScrollMargin;
	int m_iScrollInterval;
	UINT_PTR m_nTimer;
	std::vector<HTREEITEM> m_vechItemCut;
};