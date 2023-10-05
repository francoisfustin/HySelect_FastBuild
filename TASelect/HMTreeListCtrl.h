#pragma once


#include "TreeListCtrl.h"
#include "TCImageList.h"
#include "HMTreeDroptarget.h"

class CHMTreeDropTarget;
class CHMTreeListCtrl : public CTreeListCtrl
{
public:
	CHMTreeListCtrl();
	virtual ~CHMTreeListCtrl() {}

	enum eTreeOverlay
	{
		etoExclamation = 1,
		etoCrossMark = 2,
		etoMissingPV = 3,
		etoWhiteCross = 4,
		etoClosedBox = 5,
	};

	typedef struct
	{
		CDS_HydroMod *pHM;
		bool bImported;
		bool bUnused1; // these boolean are not used here but are used on classes that use HMTreeListCtrl
		bool bUnused2;
		bool bUnused3;
		bool bUnused4;
		bool bUnused5;
		bool bUnused6;
		bool bUnused7;
	}m_UserData;

	// Boolean that is true by default
	// If it's false, only hItems with m_UserData.bImported = true
	// will be dragdropped.
	bool m_bAllhItemDragDrop;

public:
	// 'fOnlyModule' is set to 'true' to display only modules (not terminal units).
	bool FillHMNode( HTREEITEM hPItem, CTable *pTab, bool fOnlyModule = false );
	
	// Allow to fill tree.
	// 'fOnlyModule' is set to 'true' to display only modules (not terminal units).
	bool FillHMRoot(CTable *pTab, CString RootName, bool fOnlyModule = false );
	
	// Allow/Deny the possibility to drag and drop on his own tree.
	void SetProperDragDrop( bool fDrag );
	
	// Allow/Deny the possibility to drag and drop all hItem or only hItem from the map (if Drag and drop on his own tree is enable).
	void SetFullDragDrop( bool bDrag ) { m_bAllhItemDragDrop = bDrag; }
	
	// Adds an item to the tree control.
	HTREEITEM AddItem( HTREEITEM hParent, CString csItemName, HTREEITEM hInsAfter, int iSelImage, int iNonSelImage, long lParam, m_UserData *pUDSource, DROPEFFECT dropEffect );
	
	// Removes the specified item from the control.
	virtual BOOL DeleteItem( _In_ HTREEITEM hItem );
	
	// Expand/Collapse the hItem and all his children.
	void ExpandRecursive( HTREEITEM hItem, UINT nCode );
	
	// Notify parent an object has been added or removed in the tree.
	void NotifyToParentTreeModified();

	// Allow to know the deep of the tree.
	// Param: 'fVisible' set to true if you want the visible deep (not the real deep of the tree but deep of visible items).
	short GetTreeDeep( bool fVisible = false );

	// Allow to check/uncheck all checkboxes on one level (0-indexed).
	// Param: 'iLevel' indicates level on which to check/uncheck.
	// Param: 'fCheck' for the checkbox state.
	// Param: 'fClearAfter' set to 'true' if we must clear checkboxes on all level below.
	// Remark: it is typically used in 'DlgDiversityFactor' with slider control.
	void SetCheckByLevel( int iLevel, bool fCheck, bool fClearAfter );

	// Allow to check/uncheck all checkboxes in all parents and children.
	// Param: 'fDoParents' is 'true' if we must apply 'fCheckParents' in all parent of the current item.
	// Param: 'fDoChildren' is 'true' if we must apply 'fCheckChildren' in all children of the current item.
	// Param: 'fCheckParents' is the check/uncheck value of all parents of the current item if 'fDoParents' is 'true'.
	// Param: 'fCheckChildren' is the check/uncheck value of all childrens of the current item if 'fDoChildren' is 'true'.
	// Remark: it is typically used in 'DlgDiversityFactor' when user directly clicks on the tree.
	void SetCheckSpecial( HTREEITEM hItem, bool fDoParents, bool fDoChildren, bool fCheckParents, bool fCheckChildren );

	// Allow to see if there is at least one checkbox checked in the tree.
	bool IsAtLeastOneChecked( void );

	void ExpandNodeAfterDelay( HTREEITEM hItem, UINT uiDelay );

protected:

	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnBegindrag( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage( MSG* pMsg );

// Private variables.
private:
	CHMTreeDropTarget m_CTreeDropTarget;
	HTREEITEM m_hItemToExpand;
	UINT_PTR m_nTimer;
};