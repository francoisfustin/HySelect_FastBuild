#pragma once


#include "DialogExt.h"

class CDlgKvSeriesTabKvCv : public CDialogExt
{
public:
	enum { IDD = IDD_DLGKVSERIESTABKVCV };

	CDlgKvSeriesTabKvCv( CWnd *pParent = NULL );

	void ResetAll( bool bKvCv ); // true for Kv, false for Cv
	double GetKvCvValue() { return m_dKvCv; }

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeKvCv();
	afx_msg void OnEnKillFocusKvCv();

protected:
	CEdit m_EditCoefficient;
	CStatic	m_StaticName;
	double	m_dKvCv;
};
