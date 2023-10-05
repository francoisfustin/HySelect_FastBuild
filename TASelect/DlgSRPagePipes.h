#pragma once


#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "HMTreeListCtrl.h"
#include "DlgCtrlPropPage.h"
#include "DlgReplacePopup.h"

class CDlgSRPagePipes : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgSRPagePipes )

public:
	enum { IDD = IDD_TABDLGSEARCHPIPES };

	CDlgSRPagePipes( CWnd *pParent = NULL );
	virtual ~CDlgSRPagePipes() {}

	// Overrides 'CDlgCtrlPropPage'.
	virtual void SetTree( LPARAM pHMTreeListCtrl );
	virtual bool OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams );

// Protected methods.
protected:
	enum eTypeInfo
	{
		eTIPipeSerie,
		eTIPipeSize,
		eTIPipes
	};

	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnTreeItemCheckStatusChanged( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnFindNextPipe( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnReplacePipe( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnReplaceAll( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnClosePopup( WPARAM wParam = 0, LPARAM lParam = 0 );

	afx_msg void OnCbnSelChangeFindPipeSerie();
	afx_msg void OnCbnSelChangeReplacePipeSerie();
	afx_msg void OnCbnSelChangeFindPipeSize();
	afx_msg void OnCbnSelChangeReplacePipeSize();
	afx_msg void OnBnClickedCheckDistPipes();
	afx_msg void OnBnClickedCheckCircPipes();
	afx_msg void OnBnClickedCheckApplyLocked();

protected:
	CExtNumEditComboBox m_CBFindPipeSerie;
	CExtNumEditComboBox m_CBFindPipeSize;
	CExtNumEditComboBox m_CBReplacePipeSerie;
	CExtNumEditComboBox m_CBReplacePipeSize;
	CButton m_BtnStart;
	CXGroupBox m_GroupFind;
	CButton m_ChBDistPipes;
	CButton m_ChBCircPipes;
	CButton m_ChBApplyLocked;
	CEdit m_EditInfo;
	
	CHMTreeListCtrl *m_pTree;
	CRank m_PipeList;
	LPARAM m_lparam;
	LPARAM m_lparamCurrent;
	CString m_strStaticFind1;
	CString m_strStaticFind2;
	CString m_strStaticReplace;

// Private methods.
private:
	void _GetPipeListFromTree( CRank *pList, eTypeInfo eInfo, HTREEITEM hItem = NULL );
	void _FillComboBoxFindPipesSerie();
	void _FillComboBoxFindPipesSize();
	void _FillComboBoxReplacePipesSerie();
	void _FillComboBoxReplacePipesSize();
	void _FillComboBox( CRank* pList, CExtNumEditComboBox *pCCombo );
	void _UpdatePopupData();
	void _UpdateEditData();
	void _UpdateStartBtn();
};
