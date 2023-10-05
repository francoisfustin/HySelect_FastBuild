#pragma once


#include "XGroupBox.h"
#include "DialogExt.h"

class CDlgKvSeriesTabDzeta : public CDialogExt
{
public:
	enum { IDD = IDD_DLGKVSERIESTABDZETA };

	CDlgKvSeriesTabDzeta( CWnd *pParent = NULL );

	void ResetAll();
	double GetIntDiam();
	void SetCheckBox( bool bState );
	double GetDzetaValue() { return m_dDzeta; }

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelChangePipeName();
	afx_msg void OnCheck();
	afx_msg void OnEnChangeDzeta();
	afx_msg void OnEnKillFocusDzeta();
	afx_msg void OnEnChangeInternalDiameter();
	afx_msg void OnEnKillFocusInternalDiameter();
	afx_msg void OnCbnSelChangePipeSize();

protected:
	CButton m_CheckInternalDiameter;
	CXGroupBox m_GroupPipe;
	CComboBox m_ComboPipeName;
	CComboBox m_ComboPipeSize;
	CEdit m_EditDzeta;
	CEdit m_EditIntDiameter;
	CStatic m_StaticUnit;
	double m_dDzeta;
	double m_dIntDiam;

// Private methods.
private:
	// Fill the 2 combo boxes of the Pipe group.
	// Pipe and Size specify the default selections. If NULL, the "Generic ..." 
	// and "** All ..." strings are respectively selected.
	void _FillComboPipes( LPCTSTR ptcPipe = NULL, LPCTSTR ptcSize = NULL );

	void _FillComboSize();
};
