#pragma once


class CDlgExtKvCvInput : public CDialogEx
{
public:
	enum { IDD = IDD_DLGKVCVINPUT };
	CDlgExtKvCvInput( CWnd *pParent = NULL );
	
	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display( double *pdKvCv, double dFlow = 0.0 );

// Public variables.
public:
	CEdit m_EditKvCv;
	CEdit m_EditFlow;
	CEdit m_EditDp;

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual afx_msg void OnChangeEditkvcv();

// Protected variables.
protected:
	double m_dFlow;
	double m_dRho;
	double *m_pdKvCv;
};