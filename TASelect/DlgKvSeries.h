#pragma once


#include "DlgKvSeriesTabNomCond.h"
#include "DlgKvSeriesTabKvCv.h"	
#include "DlgKvSeriesTabDzeta.h"	

class CDlgKvSeries : public CDialogEx
{
public:
	enum { IDD = IDD_DLGKVSERIES };

	CDlgKvSeries( CWnd *pParent = NULL );
	
	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display();
	void UpdateLstCtrl();
	void UpdateCurrentKv();

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSelChangeTabCtrl( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnButtonAdd();
	afx_msg void OnEndLabelEditList( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnButtonRemove();
	afx_msg void OnClickList( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnButtonPrint();
	afx_msg void OnDestroy();

// Private methods.
private:
	void _DisplayTabCDlg( CDialogExt *pTabCDlg );

	void _ComputeKvCv();

// Private variables.
private:
	CTADatabase *m_pTADB;
	CUnitDatabase *m_pUnitDB;
	CDlgKvSeriesTabNomCond *m_pclDlgKvSeriesTabNomCond;
	CDlgKvSeriesTabKvCv *m_pclDlgKvSeriesTabKvCv;	
	CDlgKvSeriesTabDzeta *m_pclDlgKvSeriesTabDzeta;
	CDialogExt *m_pCurCDlg;		// Currently displayed child dlg
	CFont m_font;

	CEdit m_EditCurrentKvCv;
	CStatic m_StaticKvCv;
	CStatic m_StaticTotal;
	CStatic m_StaticSum;
	CButton m_ButtonQuit;
	CButton m_ButtonAdd;
	CButton m_ButtonRemove;
	CListCtrl m_List;
	CTabCtrl m_TabCtrl;

	enum eDlgType	
	{
		NOMCOND,
		KV,
		CV,
		DZETA
	}m_CurrentDlg;
};