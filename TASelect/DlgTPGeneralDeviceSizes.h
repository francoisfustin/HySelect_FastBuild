#pragma once


#include "DlgCtrlPropPage.h"

class CDlgTPGeneralDeviceSizes : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralDeviceSizes )

public:
	enum { IDD = IDD_TABDIALOGTECHDEVSIZE };

	CDlgTPGeneralDeviceSizes( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );

// protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioSameSizeAsPipe();
	afx_msg void OnRadioSizeUpTo();
	afx_msg void OnChangeEditSizeShiftAbove();
	afx_msg void OnChangeEditSizeShiftBelow();

// Protected variables.
protected:
	CTADatastruct*	m_pTADS;
	
	CButton			m_RadioSameSizeAsPipe;
	CButton			m_RadioSizeUpTo;
	CEdit			m_EditSizeShiftAbove;
	CEdit			m_EditSizeShiftBelow;
	CSpinButtonCtrl	m_SpinSizeShiftAbove;
	CSpinButtonCtrl	m_SpinSizeShiftBelow;

	int				m_iSizeShift;
	int				m_iSizeShiftAbove;
	int				m_iSizeShiftBelow;
};