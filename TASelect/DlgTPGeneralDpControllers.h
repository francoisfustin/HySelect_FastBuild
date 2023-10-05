#pragma once


#include "DlgCtrlPropPage.h"
#include "ExtNumEdit.h"
#include "afxwin.h"

class CDlgTPGeneralDpControllers : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralDpControllers )

public:
	enum { IDD = IDD_TABDIALOGTECHDPC };

	CDlgTPGeneralDpControllers( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString SectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckHide();
	afx_msg void OnBnClickedSameSize();

// Protected variables.
protected:
	CTADatastruct*	m_pTADS;
	
	CExtNumEdit		m_EditMinDp;
	CButton			m_CheckHide;
	CButton			m_CheckMvWithSameSize;
	
	double			m_dDpCMinDp;
	int				m_iDpCHide;
	int				m_iDpCMvWithSameSize;
};
