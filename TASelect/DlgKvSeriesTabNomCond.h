#pragma once


#include "XGroupBox.h"
#include "DialogExt.h"

class CDlgKvSeriesTabNomCond : public CDialogExt
{
public:
	enum { IDD = IDD_DLGKVSERIESTABNOMCOND };

	CDlgKvSeriesTabNomCond( CWnd *pParent = NULL );

	void SetWaterChar();
	void ResetAll();

	void GetFlowText( CString &str ) { return m_EditFlow.GetWindowText( str ); }
	void GetDpText( CString &str ) { return m_EditFlow.GetWindowText( str ); }
	double GetFlowValue() { return m_dFlow; }
	double GetDpValue() { return m_dDp; }

	CWaterChar GetCurrentWaterChar() { return m_clWaterChar; }
	
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeFlow();
	afx_msg void OnEnKillFocusFlow();
	afx_msg void OnEnChangeDp();
	afx_msg void OnEnKillFocusDp();	
	afx_msg void OnButtonModWater();

protected:
	CWaterChar m_clWaterChar;
	ProjectType m_eCurrentProjectType;
	CEdit m_EditFlow;
	CEdit m_EditDp;
	CXGroupBox m_GroupWater;
	CMFCButton m_ButtonModWater;
	CStatic m_StaticQUnit;
	CStatic m_StaticPdUnit;
	double m_dFlow;
	double m_dDp;
};