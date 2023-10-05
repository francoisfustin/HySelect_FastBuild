#pragma once


#include "MyToolTipCtrl.h"
#include "ExtComboBox.h"
#include "XGroupBox.h"

class CDlgViscCorr : public CDialogEx
{
public:
	enum { IDD = IDD_DLGVISCCORR };

	CDlgViscCorr( CWnd *pParent = NULL );

	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg void OnButtonModWater();
	afx_msg void OnSelChangeComboExType();
	afx_msg void OnSelChangeComboFamily();
	afx_msg void OnSelChangeComboDevice();
	afx_msg void OnRadioSetting();
	afx_msg void OnRadioDp();
	afx_msg void OnRadioChartFlow();
	afx_msg void OnRadioTrueFlow();
	afx_msg void OnChangeEditSetting();
	afx_msg void OnChangeEditDp();
	afx_msg void OnChangeEditChartFlow();
	afx_msg void OnChangeEditTrueFlow();
	afx_msg void OnKillFocusEditSetting();
	afx_msg void OnKillFocusEditDp();
	afx_msg void OnKillFocusEditChartFlow();
	afx_msg void OnKillFocusEditTrueFlow();

// Protected variables.
protected:
	CWaterChar m_clWaterChar;
	ProjectType m_eCurrentProjectType;
	CMFCButton m_RadioTrueFlow;
	CMFCButton m_RadioDp;
	CMFCButton m_RadioChartFlow;
	CMFCButton m_RadioSetting;
	CMFCButton m_ButtonModWater;
	CEdit m_EditTrueFlow;
	CEdit m_EditSetting;
	CEdit m_EditDp;
	CEdit m_EditChartFlow;
	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboDevice;
	CExtNumEditComboBox m_ComboFamily;
	CXGroupBox m_GroupWater;
	CXGroupBox m_GroupValve;
	CXGroupBox m_GroupRes;
	HICON m_HICONAddit;
	CTADatabase *m_pTADB;				// Database containing TA products and pipes
	CTADatastruct *m_pTADS;				// Data structure constituting the document
	CMyToolTipCtrl m_ToolTip;			// ToolTipCtrl
	CImageList m_ImageListComboType;	// Image list for ComboEx product type
	CString m_strSelType;					// Selected type
	CEnBitmap m_BitmapIn;					// Bitmap for down state of radio buttons
	CEnBitmap m_BitmapOut;				// Bitmap for up state of radio buttons
	CMFCButton *m_pPushed1;				// Pointer on 1st of the two last pushed radio button
	CMFCButton *m_pPushed2;				// Pointer on the last pushed radio button
	CEdit *m_pEditIn1;					// Pointer on edit box corresponding to m_Pushed1
	CEdit *m_pEditIn2;					// Pointer on edit box corresponding to m_Pushed2
	CEdit *m_pEditOut1;					// Pointer on output edit box
	CEdit *m_pEditOut2;					// Pointer on output edit box
	double m_dSetting;
	double m_dDp;
	double m_dChartFlow;
	double m_dTrueFlow;
	CDB_RegulatingValve *m_pDev;
	double m_dMinOpening;
	double m_dMaxOpening;

// Private methods.
private:
	// Fill the Valve type Comboex box.
	// Type specifies the default selection. If NULL, the "** All ..." string is selected.
	void _FillComboexType( LPCTSTR ptcType = NULL );
	
	// Retrieve the product type ID which is selected in the type ComboEx.
	LPCTSTR _GetSelTypeID();
	
	// Retrieve the product family ID which is selected in the family Combo.
	LPCTSTR _GetSelFamilyID();
	
	// Update all fields.
	void _UpdateAll();
	
	// Get the second output edit box, the other being pEditOut1.
	CEdit *_Get2ndOutEdit( CEdit *pEditOut1 );
	
	// Swap button pointer and edit pointer if there are already checked 
	// and the user push on the button pointer 1.
	bool _SwapCtrl( CButton *pButton );
	
	// Fill correct value to edit pointer in function of pushed buttons.
	void _SetEditvsButton();
};