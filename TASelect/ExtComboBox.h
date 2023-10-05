#pragma once


#include "ExtEdit.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "DataBase.h"

class CExtComboBoxDBTools
{
public:
	CExtComboBoxDBTools( CComboBox *pCB );
	virtual ~CExtComboBoxDBTools() {}

	const IDPTR GetCBCurSelIDPtr();
	void SetCurSelWithID( CString strID );

	// Returns -1 if not found otherwise returns the index.
	const int FindCBIDPtr( CString strID );

	const int FindItemDataPtr( DWORD_PTR dwItemToFind );
	void FillInCombo( CRankEx *pList, CString strID, int iAllIds = 0, bool bIsCustomized = false );

protected:
	CComboBox *m_pCB;
};

///////////////////////////////////////////////////////////
//
//						CExtNumEditComboBox
//
// This class can allow to dropdown a list of choice but also 
// to be able to enter numerical data (As the 'Kvs' combo in the
// control valve individual selection).
///////////////////////////////////////////////////////////

class CExtNumEditComboBox : public CComboBox, public CExtComboBoxDBTools
{
	DECLARE_DYNAMIC(CExtNumEditComboBox)

public:
	CExtNumEditComboBox();
	virtual ~CExtNumEditComboBox();

	void SetBackColor( COLORREF rgb );
	COLORREF GetBackColor( void );

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	afx_msg HBRUSH CtlColor( CDC *pDC, UINT nCtlColor );
	afx_msg void OnCbnDropdown();
	afx_msg void OnDestroy();

private:
	COLORREF m_crBackGnd;
	CBrush m_brBackGnd;
	CExtNumEdit m_clExtNumEdit;
	CExtStatic m_clExtStatic;
};

///////////////////////////////////////////////////////////
//
//						CExtEditComboBox
//
// This class can allow to dropdown a list of choice but also 
// to be able to enter text data (As the remark combo in the
// confirmation selection dialog).
///////////////////////////////////////////////////////////

class CExtEditComboBox : public CComboBox, public CExtComboBoxDBTools
{
	DECLARE_DYNAMIC( CExtEditComboBox )

public:
	CExtEditComboBox();
	virtual ~CExtEditComboBox();

	void SetBackColor( COLORREF rgb );
	COLORREF GetBackColor( void );

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	afx_msg HBRUSH CtlColor( CDC *pDC, UINT nCtlColor );
	afx_msg void OnCbnDropdown();
	afx_msg void OnDestroy();

private:
	COLORREF m_crBackGnd;
	CBrush m_brBackGnd;
	CExtEdit m_clExtEdit;
	CExtStatic m_clExtStatic;
};

///////////////////////////////////////////////////////////
//
//						CExtComboBoxEx
//
///////////////////////////////////////////////////////////
#define CX_BITMAP 24
#define CY_BITMAP 24
class CExtComboBoxEx : public CComboBoxEx, public CExtComboBoxDBTools
{
	DECLARE_DYNAMIC( CExtComboBoxEx )

public:
	CExtComboBoxEx();
	virtual ~CExtComboBoxEx() {}
	bool EnableWindowW( bool bEnable = true, bool bRecurseChildren = false );
	CImageList *SetImageListW( CImageList *pclImageList, bool bEnabledMode = true );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnDropdown();

	// To add style 'CBS_OWNERDRAWFIXED' to the combo.
	virtual void PreSubclassWindow();

	// If a combo box has the style 'CBS_OWNERDRAWFIXED' defined, 'DrawItem' is called BUT for the parent of the control.
	// If we don't want to implement in each dialog, we intercept message here.
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	
private:
	void _DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

private:
	bool m_fIsEnabled;
	CImageList *m_pclImageListNormal;
	CImageList *m_pclImageListGrayed;
};
