#pragma once


#include "DrawSet.h"
#include "FormViewEx.h"

class CDS_HydroMod;
class CSheetHMCalc;
class CRViewHMCalc : public CFormViewEx, protected CDrawSet
{
DECLARE_DYNCREATE( CRViewHMCalc )

public:
	enum { IDD = IDV_RVIEW };

	typedef enum LastOperation
	{
		Undefined = -1,
		Cut,
		Copy
	};

	CRViewHMCalc();
	virtual ~CRViewHMCalc();

// Public methods.
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );

	void Reset( void );
	void EnableOnDraw( bool bEnable ) { m_bEnableOnDraw = bEnable; }
	void RedrawHM( CDS_HydroMod *pHM = NULL );
	void EditHM( CDS_HydroMod *pHM );
	CSheetHMCalc* GetSheetHMCalc() { return m_pSheetHMCalc; }
	
	// Allow to set what was the last operation (Cut or copy).
	// Remark: It's to help us when pasting.
	void SetLastCopyCutOperation( LastOperation eLastOperation );
	
	// Allow to know what was the last operation (Cut or copy).
	LastOperation GetLastCopyCutOperation( void );

	// Allow to change display status of HydroMod in the 'SheetHMCalc' and in 'TabCDialogProj'.
	// Remark: when 'Cut' or 'Copy' occurs, it's better to call this method to change display status. Because here
	//         we are sure to call both the sheet in 'SheetHMCalc' and the tree in 'TabCDialogProj'.
	void SetFlagCut( std::vector<CDS_HydroMod*>* pvecHydromod );

	// Allow to reset all lines displayed in cut mode.
	// Returns 'true' is at least one item as been changed in 'SheetHMCalc'.
	// Remark: same remark as above.
	bool ResetFlagCut( void );

	// Allow to know if there at least one item in the tree that is in a cut mode.
	// Remark: same remark as above.
	bool IsFlagCutSet( void );

	// Allow to check if an object is in cut mode or not.
	// Remark: same remark as above.
	bool IsObjectInCutMode( CData* pObject );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
	virtual void OnDraw( CDC* pDC );
	
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnKillFocus( CWnd* pNewWnd );
	afx_msg void OnDestroyClipboard();
	afx_msg void OnDestroy(); 

	afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

	afx_msg void OnFltMenuTabProjCreateIn();
	afx_msg void OnFltMenuTabProjCreateBefore();
	afx_msg void OnFltMenuTabProjAddValve();
	afx_msg void OnFltMenuTabProjEdit();
	afx_msg void OnFltMenuTabProjRename();
	afx_msg void OnFltMenuTabProjDelete();
	afx_msg void OnFltMenuTabProjCut();
	afx_msg void OnFltMenuTabProjCopy();
	afx_msg void OnFltMenuTabProjPasteIn();
	afx_msg void OnFltMenuTabProjPasteBefore();
	afx_msg void OnFltMenuTabProjPrint();
	afx_msg void OnFltMenuTabProjPrintRecursive();
	afx_msg void OnFltMenuTabProjExport();
	afx_msg void OnFltMenuTabProjExportRecursive();

	afx_msg void OnFltMenuRVHMCalcAddCircuit();
	afx_msg void OnFltMenuRVHMCalcEdit();
	afx_msg void OnFltMenuRVHMCalcRename();
	afx_msg void OnFltMenuRVHMCalcDelete();
	afx_msg void OnFltMenuRVHMCalcCut();
	afx_msg void OnFltMenuRVHMCalcCopy();
	afx_msg void OnFltMenuRVHMCalcPasteIn();
	afx_msg void OnFltMenuRVHMCalcPasteBefore();
	afx_msg void OnFltMenuRVHMCalcEditAccessories();
	afx_msg void OnFltMenuRVHMCalcCopyAccessories();
	afx_msg void OnFltMenuRVHMCalcPasteAccessories();
	afx_msg void OnFltMenuRVHMCalcLock();
	afx_msg void OnFltMenuRVHMCalcUnLock();
	afx_msg void OnFltMenuRVHMCalcLockColumn();
	afx_msg void OnFltMenuRVHMCalcUnlockColumn();
	afx_msg void OnFltMenuRVHMCalcLockAll();
	afx_msg void OnFltMenuRVHMCalcUnlockAll();
	afx_msg void OnFltMenuRVHMCalcPrint();
	afx_msg void OnFltMenuRVHMCalcPrintRecursive();
	afx_msg void OnFltMenuRVHMCalcExport();
	afx_msg void OnFltMenuRVHMCalcExportRecursive();
	afx_msg void OnFltMenuRVHMCalcGetFullinfo();

	afx_msg void OnUpdateMenuText( CCmdUI* pCmdUI );

	// Spread DLL message handlers.
	// Remark: Why implementing these methods in the 'CRViewHMCalc' class and not directly in the 'CSheetHMCalc' class?
	//         When the '_TSpread' class sends the 'LeaveCell' event, this one is directly sent to the 'CSheetHMCalc' class
	//         and also to its parent (here the 'CRViewHMCalc' class). If we implement handlers only in the 'CSpreadComboBox', 
	//		   class this, this one will return 'TRUE' or 'FALSE' in regards to its need. After that, the same event is sent
	//         to the owner of the 'CSheetHMCalc' class that is the 'CRViewHMCalc' class. By default, if there is no 
	//         handler in the 'CRViewHMCalc' class, the return value will automatically be set to 'FALSE' and then will
	//         overwrite the previous value set by the 'CSheetHMCalc' class. This is why we voluntary don't catch
	//         '_TSpread' event in the 'CSheetHMCalc' class, but we do it only in 'CRViewHMCalc'.
	afx_msg LRESULT ComboDropDown( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT DragDropBlock( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT EditChange( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT LeaveCell( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT SheetChanged( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT TextTipFetch( WPARAM wParam, LPARAM lParam );

// Protected variables.
protected:
	CSheetHMCalc *m_pSheetHMCalc;
	bool m_bEnableOnDraw;
	bool m_bCopyDistribAccess;
	long m_lRow;
	long m_lColumn;
	int m_iSpreadYPos;
	CDB_TAProduct *m_pTAPForFullCatalog;
	CMenu m_FltMenuTabProj;
	CMenu m_FltMenuRVHMCalc;
	bool m_bActivateContextMenu;
	LastOperation m_eLastOperation;
	std::map<UINT, CString> m_mapUpdateMenuTooltip;

// Private methods.
private:
	typedef enum CreateMode
	{
		CreateIn,
		CreateBefore
	};
	void _TabProjCreate( CreateMode eCreateMode );

	void _OnContextMenuDelete();
	void _OnContextMenuCopy();

	typedef enum PasteMode
	{
		PasteIn,
		PasteBefore
	};
	void _OnContextMenuPaste( PasteMode ePasteMode );
	
	void _OnContextMenuPrint();
	void _OnContextMenuPrintRecursive();
	void _OnContextMenuExport();
	void _OnContextMenuExportRecursive();
};

extern CRViewHMCalc *pRViewHMCalc;
