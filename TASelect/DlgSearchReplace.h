#pragma once


#include "TCImageList.h"
#include "MyToolTipCtrl.h"
#include "DlgSRPagePipes.h"
#include "DlgSRPageTemp.h"
#include "DlgSRPageCircuit.h"
#include "DlgSRPageSet.h"
#include "DlgSRPageActuator.h"
#include "Hydromod.h"
#include "DlgCtrlPropPageMng.h"
#include "ExtNumEdit.h"

class CDlgSearchReplace : public CDlgCtrlPropPageMng, public CDlgSRPageSet::IDlgSRPageSetNotificationHandler,
	public CDlgSRPageActuator::IDlgSRPageActuatorNotificationHandler
{
	DECLARE_DYNAMIC( CDlgSearchReplace )

public:
	enum OpenFromWhere
	{
		OFW_RibbonProject,
		OFW_RibbonProductSel
	};
	enum OpeningPage
	{
		OP_HMPipes,
		OP_HMTemp,
		OP_HMCircuit,
		OP_SetGroupOrSplit,
		OP_ActrAddOrRemove,
	};

	enum RadioState
	{
		RS_HMCalc = 0,
		RS_IndSel,
		RS_Uninitialized = 0xFF
	};

	enum ControlValveType
	{
		CVT_CV,						// For 'DlgSearchAndReplaceSet' and 'DlgSearchAndReplaceActuator'.
		CVT_BCV,					// For 'DlgSearchAndReplaceSet' and 'DlgSearchAndReplaceActuator'.
		CVT_PICV,					// For 'DlgSearchAndReplaceSet' and 'DlgSearchAndReplaceActuator'.
		CVT_LastSet,
		CVT_TRV = CVT_LastSet,		// ONLY for 'DlgSearchAndReplaceActuator'.
		CVT_FLCV,					// ONLY for 'DlgSearchAndReplaceActuator'.
		CVT_Last
	};

	enum { IDD = IDD_DLGSEARCHREPLACE };

	CDlgSearchReplace( OpenFromWhere eOpenFromWhere, CWnd *pParent = NULL );
	virtual ~CDlgSearchReplace();

	CHMTreeListCtrl *GetpTree()
	{
		return &m_Tree;
	}
	HTREEITEM GetHItem( CDS_HydroMod *pHMToFind, HTREEITEM hTreeItem = NULL );
	void ResetItemsColor( COLORREF col, HTREEITEM hTreeItem = NULL );

	int  GetSelectedModules( std::map<int, CDS_HydroMod *> *pMap, int &iPos, CDS_HydroMod::eHMObj fLockCircuits = CDS_HydroMod::eHMObj::eNone, BOOL fModules = TRUE,
		 BOOL fCircuits = TRUE, HTREEITEM hTreeItem = NULL );
	
	void UpdateTreeHMpointers( std::map<WPARAM, CDS_HydroMod *> *pMap, HTREEITEM hTreeItem = NULL );
	
	void EnableButtonStart( BOOL fEnable ) { m_BtnStart.EnableWindow( fEnable ); }

	CRect GetLeftTreeWidth( void );

	// Overrides 'CDlgSRPageSet::IDlgSRPageSetNotificationHandler'.
	virtual void SRPageSet_OnRadioSplitClicked();
	virtual void SRPageSet_OnRadioGroupClicked();
	virtual void SRPageSet_OnReplacePopupHM( CDS_HydroMod *pHM );

	// Overrides 'CDlgSRPageActuator::IDlgSRPageActuatorNotificationHandler'.
	virtual void SRPageActuator_OnRadioRemoveClicked();
	virtual void SRPageActuator_OnRadioAddClicked();
	virtual void SRPageActuator_OnCheckRemoveForSet();
	virtual void SRPageActuator_OnReplacePopupHM( CDS_HydroMod *pHM );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadios( UINT nID );
	afx_msg void OnCbnSelChange();
	afx_msg void OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnBnClickedCheckAll();
	afx_msg void OnBnClickedUncheckAll();
	afx_msg void OnBnClickedExpandAll();
	afx_msg void OnBnClickedCollapseAll();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedCancel();

	// Protected variables.
protected:
	CHMTreeListCtrl m_Tree;
	CComboBox m_ComboBox;
	CExtStatic m_ExtStaticHeader;
	CMFCButton m_BtnCheck;
	CMFCButton m_BtnUnCheck;
	CMFCButton m_BtnExpand;
	CMFCButton m_BtnCollapse;
	CMyToolTipCtrl m_ToolTip;
	CButton m_BtnStart;

	// Private methods.
private:
	void _InitializeLayout( OpeningPage eOpeningPage );
	void _ReInitializeLayout( void );

	void _CreateTree( void );
	void _SetItemStateToDefault( HTREEITEM hItem );

	void _DisplayLeftPane( CDlgCtrlPropPage *pPage );
	bool _AddPageToComboBox( CDlgCtrlPropPage *pPage );
	void _PrepareParameters( void );

	// Methods specific for the hydraulic network mode.
	void _InitLayoutHMCalc( void );
	void _PrepareForSetHM( HTREEITEM hItem );
	void _PrepareForActuatorHM( HTREEITEM hItem );
	void _CheckAllHMChildren( HTREEITEM hItem, bool fCheck );

	// Methods specific for the individual selection mode.
	void _InitLayoutIndSel( void );
	void _PrepareForSetIndSel( void );
	void _PrepareForActuatorIndSel( void );
	void _ClickOnControlValveType( HTREEITEM hItem );
	void _CheckAllControlValveType( bool fCheck );

	// Helper methods.
	bool _IsCheckboxEnabled( HTREEITEM hItem );
	void _SetCheckboxEnable( HTREEITEM hItem, bool fEnable );
	bool _IsCheckboxChecked( HTREEITEM hItem );
	void _SetCheckboxCheck( HTREEITEM hItem, bool fCheck );
	bool _IsAtLeastOneCheckedEnabled( HTREEITEM hItem );
	bool _IsAllChecked( HTREEITEM hItem );
	bool _IsAllUnchecked( HTREEITEM hItem );
	bool _IsAllEnabled( HTREEITEM hItem );
	bool _IsAllDisabled( HTREEITEM hItem );
	void _VerifyRootItem( void );
	void _VerifyButtonStart( void );

	class CMyRadioButton : public CButton
	{
		// Remark: We don't use 'CButton' class for radio buttons in the three hydronic calculation dialogs.
		//         Usually when we need radio button, we create a group and put radio buttons in it. First button must have 'Group' and 'Tab stop'
		//         properties set to 'TRUE' and these properties must be set to 'FALSE' for others.
		//         If we let MFC to automatically manage these buttons, when playing with TAB key, focus can bet set on the active radio button
		//         (just at the right of the radio button but just before text area even if text is empty!).
		//         Event if we create three separate buttons and we disable 'tab stop' property for them, it is yet possible to have focus when clicking
		//         on the button. For this, we just use this class to override 'OnSetFocus' to do nothing with it.
		DECLARE_MESSAGE_MAP()
		afx_msg void OnSetFocus( CWnd *pOldWnd );
	};

	// Private variables.
private:
	OpenFromWhere m_eOpenFromWhere;

	CMyRadioButton m_RadioHMCalc;
	CMyRadioButton m_RadioIndSel;
	RadioState m_eRadioState;
	OpeningPage m_eCurrentPage;

	CDlgSRPagePipes m_PagePipes;
	CDlgSRPageTemp m_PageTemp;
	CDlgSRPageCircuit m_PageCircuit;
	CDlgSRPageSet m_PageSet;
	CDlgSRPageActuator m_PageActuator;

	CDS_HydroMod *m_pHMHighlighted;

	CImageList *m_pclImgListCheckBox;

	std::vector<CDS_HydroMod *> m_vecSelectedCicuits;

	bool m_bIndSelTreeInitialized;
	bool m_bHMTreeInitialized;
	std::map<ControlValveType, std::vector<CDS_SSelCtrl *>> m_mapControlValveList;
	std::map<ControlValveType, bool> m_mapCVCheckboxEnableState;
	std::map<ControlValveType, bool> m_mapCVCheckboxCheckState;
	std::vector<CDS_SSelCtrl *> m_vecSSelCtrl;

	std::vector<LPARAM> m_vecParams;
};

extern CDlgSearchReplace *pDlgSearchReplace;
