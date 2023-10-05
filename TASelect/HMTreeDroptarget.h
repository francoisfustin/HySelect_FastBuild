#pragma once

class CHMTreeListCtrl;
class CHMTreeDropTarget :public COleDropTarget	
{
public:
	CHMTreeDropTarget();

	// Allow/Deny the possibility to drag and drop on his own tree.
	void SetProperDragDrop( bool fDrag ) { m_bAllowProperDragDrop = fDrag; }

	virtual DROPEFFECT OnDragEnter( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	virtual DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	virtual void OnDragLeave( CWnd* pWnd );
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

// Private methods.
private:
	void _RetrieveSource( COleDataObject* pDataObject );
	void _FreeMemory( COleDataObject* pDataObject );

	// Recursive function for adding items from source tree to destination tree.
	void _AddItem( HTREEITEM hSrcTItem, HTREEITEM hDestTItem, DROPEFFECT dropEffect );
	
	// Function to get the index of the item’s image and its selected image within the tree control’s image list.
	void _GetItemImages( HTREEITEM hSrcTItem, int &nSelItem, int &nNonSelItem );
	
// Private variables.
private:
	// Pointer to source tree control.
	CHMTreeListCtrl *m_pSourceTreeCtrl;
	
	// Pointer to destination tree control.
	CHMTreeListCtrl *m_pDestTreeCtrl;

	// Rectangle position in the tree that represent the line.
	CRect m_RectLine;

	// Marker used to insert a tree hItem.
	HTREEITEM m_MarkerNexthItem;

	// Define the hItem to expand after a period of time.
	HTREEITEM m_hTItemToExpand;

	HTREEITEM m_hDestItem;
	
	// Boolean that tells if the drag and drop is allowed on the same object(tree).
	bool m_bAllowProperDragDrop;
};
