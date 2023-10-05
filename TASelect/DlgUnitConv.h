#pragma once


#include "ExtComboBox.h"
#include "XGroupBox.h"

class CDlgUnitConv : public CDialogEx
{
public:
	enum { IDD = IDD_DLGUNITCONV };

	CDlgUnitConv( CWnd *pParent = NULL );

	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCbnSelChangePhysicalType();
	afx_msg void OnCbnSelChangeUnit1();
	afx_msg void OnCbnSelChangeUnit2();
	afx_msg void OnChangeEditValue1();
	afx_msg void OnChangeEditValue2();
	afx_msg void OnKillFocusEditValue1();
	afx_msg void OnKillFocusEditValue2();

protected:
	CStatic m_StaticLrArrow;
	CXGroupBox m_Group1;
	CXGroupBox m_Group2;
	CEdit m_EditValue1;
	CEdit m_EditValue2;
	CStatic m_StaticUnit1;
	CStatic m_StaticUnit2;
	CExtNumEditComboBox m_ComboUnit1;
	CExtNumEditComboBox m_ComboUnit2;
	CExtNumEditComboBox m_ComboPhysType;

// Private methods.
private:
	// Fill 'Physical type' combo box.
	void _FillComboPhysicalType();
	
	// Fill 'Unit1' and 'Unit2' combo boxes.
	void _FillComboUnit12();
	
	// Fill edit box 2 with converted value from edit box 1.
	void _Convert1to2();
	
	// Fill edit box 1 with converted value from edit box 2.
	void _Convert2to1();
	
	// Set the text in 'Unit1' static.
	void _SetStaticUnit1();
	
	// Set the text in 'Unit2' static.
	void _SetStaticUnit2();

// Private variables.
private:
	CUnitDatabase *m_pUnitDB;
	CTADatabase *m_pTADB;
	int m_iPhysType;			// PhysType index.
	int m_iUnit1;				// Unit1 index.
	int m_iUnit2;				// Unit2 index;
	bool m_bModified;			// Flag indicating that a selection has been modified .
	CBitmap m_BitmapLrArrow;	// Bitmap to be mapped loaded in m_StaticLrArrow;
};