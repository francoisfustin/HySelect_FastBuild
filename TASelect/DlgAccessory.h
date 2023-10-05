#pragma once


#include "afxwin.h"
#include "afxcmn.h"
#include "ExtComboBox.h"
#include "ExtListCtrl.h"
#include "XGroupBox.h"


class CDS_HydroMod;
class CPipes;
class CDlgAccessory : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgAccessory )

public:
	enum { IDD = IDD_DLGACCESSORIES };

	CDlgAccessory( CTADatabase::FilterSelection eFilterSelection, CWnd* pParent = NULL );
	virtual ~CDlgAccessory();

	INT_PTR Display( CDS_HydroMod *pHM, CPipes *pPipe, bool* pfChangeDone = NULL );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnNMClickList( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnCbnSelChangeAccessoryType();
	afx_msg void OnEnKillFocusEdit1();
	afx_msg void OnEnKillFocusEdit2();
	afx_msg void OnBnClickedCheck();
	afx_msg void OnCbnSelChangePipeSeries();
	afx_msg void OnCbnSelChangePipeSize();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedModify();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	void RefreshListCtrl();
	void FillComboPipeSeries();
	void FillComboPipeSize();

// Protected variables.
protected:
	enum eColHeader
	{
		Nb,
		Desc,
		Value,
		Dp
	};

	CTADatabase::FilterSelection m_eFilterSelection;
	CDS_HydroMod*	m_pHM;
	CPipes*			m_pPipe;
	bool*			m_pfChangeDone;
	CTADatabase*	m_pTADB;
	CRect			m_RectStatic1;
	CEdit			m_EditDescription;
	CEdit			m_Edit1;
	CEdit			m_Edit2;
	CEdit			m_EditNumberToAdd;
	CSpinButtonCtrl m_SpinNumberToAdd;
	CButton			m_CheckBox;
	bool			m_fCheckBox;
	CExtNumEditComboBox m_ComboAccessType;
	CExtNumEditComboBox m_ComboPipeSeries;
	CExtNumEditComboBox m_ComboPipeSize;
	CButton			m_ButtonAdd;
	CButton			m_ButtonModify;
	CButton			m_ButtonRemove;
	CExtListCtrl	m_List;
	CString			m_strQrefDPref;
	CString			m_strKV;
	CString			m_strCV;
	CString			m_strZeta;
	CString			m_strPipe;
	int				m_iSelItem;
	CPipes*			m_pSavedPipe;
	double			m_dQRef;
	double			m_dDpRef;
	double			m_dKv;
	double			m_dCv;
	double			m_dZeta;
	double			m_dLength;
	IDPTR			m_IDPTRPipe;
	CStatic			m_StaticTotalDp;
	CXGroupBox		m_GroupPipe;
	CXGroupBox		m_GroupAccessory;
};
