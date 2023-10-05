#pragma once

#include "CEditString.h"

class CMFCRibbonRichEditCtrlNum;
class CMFCRibbonRichEditCtrlCbiRestr;
class CMFCRibbonSpinButtonCtrlEx;
class CMFCRibbonEditEx : public CMFCRibbonEdit
{
public:
	typedef enum eTypeRichEditCtrl
	{
		eNumericalEditCtrl,
		eCBIRestrEditCtrl,
		eLastEditCtrl
	};

	CMFCRibbonEditEx( UINT nID, int nWidth, LPCTSTR lpszlabel = 0, int nImage = -1, eTypeRichEditCtrl eTypeEditCtrl = eNumericalEditCtrl );
	virtual CMFCRibbonRichEditCtrl* CreateEdit( CWnd* pWndParent, DWORD dwEditStyle );

	// Functions for numerical edit.
	void SetPhysicalTypeEdit( ePHYSTYPE eType );
	void SetPhysicalTypeEdit( CString strType );
	ePHYSTYPE GetPhysicalTypeEdit();
	void SetEditSign( CNumString::eEditSign eSign );

	// Set max value only for a double.
	void SetMaxDblValue( double dMaxVal );
	void SetMinDblValue( double dMinVal );

	// Set min and max value only for a int.
	void SetMinIntValue( int iMinVal );
	void SetMaxIntValue( int iMaxVal );

	void SetEditType( CNumString::eEditType EdiType = CNumString::eDouble, CNumString::eEditSign EditSign = CNumString::eBoth );

	// 'SetEditTextEx' do the same function as 'SetEditText' and initialize the SIValue.
	void SetEditTextEx( CString strText, bool bSIValue = false );
	void SetCurrentValSI( double dSIValue );

	LPCTSTR GetCurrentString();
	double GetCurrentValSI();

	void SetUnitsUsed( bool bUnitUsed = true );

	void SetEditForTemperature();
	CMFCRibbonRichEditCtrlNum* GetNumericalEdit() { return m_peditNum; }

	BOOL CreateSpinButtonEx( CMFCRibbonRichEditCtrl* pWndEdit, CWnd* pWndParent );
	virtual void DestroyCtrl();

// Protected methods.
protected:
	virtual void OnSetFocus( BOOL B );

protected:
	CMFCRibbonEditEx();
	virtual ~CMFCRibbonEditEx();

	eTypeRichEditCtrl				m_TypeRichEditCtrl;
	CMFCRibbonRichEditCtrlNum*		m_peditNum;
	CMFCRibbonRichEditCtrlCbiRestr*	m_peditCbiRest;
	CMFCRibbonSpinButtonCtrlEx*		m_pWndSpinEx;
};

class CMFCRibbonRichEditCtrlNum : public CMFCRibbonRichEditCtrl, public CNumString
{
// 	friend class CMFCRibbonBaseElement;
// 	friend class CNumString;
public:
	CMFCRibbonRichEditCtrlNum( CMFCRibbonEditEx& edit );
	void SetEditForTemperature() { m_bEditForTemp = true; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	afx_msg void OnSetFocus( CWnd* pOldWnd );
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnKillFocus( CWnd* pNewWnd );
	afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );
	afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

protected:
	bool m_bEditForTemp;
};

class CMFCRibbonRichEditCtrlCbiRestr : public CMFCRibbonRichEditCtrl, public CCbiRestrString
{
public:
	CMFCRibbonRichEditCtrlCbiRestr( CMFCRibbonEditEx& edit );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );
};

class CMFCRibbonSpinButtonCtrlEx : public CMFCSpinButtonCtrl
{
public:
	CMFCRibbonSpinButtonCtrlEx( CMFCRibbonEditEx* pEdit = NULL );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnDraw( CDC* pDC );
	afx_msg void OnDeltapos( NMHDR* pNMHDR, LRESULT* pResult );

protected:
	BOOL m_bQuickAccessMode;
	CMFCRibbonEditEx* m_pEdit;
};
