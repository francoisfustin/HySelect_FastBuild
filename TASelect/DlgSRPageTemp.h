#pragma once


#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "HMTreeListCtrl.h"
#include "DlgCtrlPropPage.h"
#include "DlgReplacePopup.h"

class CDlgSRPageTemp : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgSRPageTemp )

public:
	enum { IDD = IDD_TABDLGSEARCHTEMP };

	CDlgSRPageTemp( CWnd *pParent = NULL );
	virtual ~CDlgSRPageTemp() {}

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
	afx_msg LRESULT OnFindNextInjCircuit( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnReplaceTemperature(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnReplaceAll( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnClosePopup( WPARAM wParam = 0, LPARAM lParam = 0 );

	afx_msg void OnCbnSelChangeFindTemperature();
	afx_msg void OnEditChangeSupplyTemperature();

	void _ListHTreeItem( HTREEITEM hItem = NULL );
	void CheckTempEditIsValid( CString &strValue, CEdit &ceTemp );

	afx_msg void OnEditChangeReturnTemperature();

protected:
	CExtNumEditComboBox m_CBFindTemperature;
	CEdit m_EditReplaceSupplyTemp;
	CEdit m_EditReplaceReturnTemp;
	CButton m_BtnStart;
	CEdit m_EditInfo;

	CHMTreeListCtrl	*m_pTree;
	std::vector<HTREEITEM> m_vHTreeItem;
	HTREEITEM m_hTreeItem;
	CString m_strStaticFind1;
	CString m_strStaticFind2;
	CString m_strStaticReplace;

	// Private methods.
private:
	void _GetTempListFromTree( std::vector<std::pair<double, double> > &vPairTemperature, HTREEITEM hItem = NULL );
	void _FillComboBoxFindTemperatures();
	void _FillComboBox( std::vector<std::pair<double, double> > &vPairTemperature, CExtNumEditComboBox *pCCombo );
	void _UpdatePopupData();
	void _UpdateEditData();
	void _UpdateStartBtn();
};
