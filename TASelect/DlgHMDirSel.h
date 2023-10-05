#pragma once


#include "ExtComboBox.h"
#include "ExtNumEdit.h"

class CDlgHMDirSel : public CDialogEx
{
public:
	enum { IDD = IDD_DLGHMDIRSEL };

	CDlgHMDirSel( CWnd* pParent = NULL );

	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display( CString SelectedID = _T(""), double dPresetting = -1.0 );
	int Display( CString CBIType, CString CBISize, double dPresetting = -1.0 );
	CString GetSelThingID() { return m_SelThingID; }
	CString GetCategoryTab() { return _T("REGVALV_TAB"); }
	double GetPresetting() { return m_dPresetting; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeComboExType();
	afx_msg void OnSelChangeComboFamily();
	afx_msg void OnSelChangeComboDevice();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnCheckPresetting();
	afx_msg void OnChangeEditPresetting();
	afx_msg void OnItemChangedListDevice( NMHDR* pNMHDR, LRESULT* pResult );

// Protected variables.
protected:
	CListCtrl m_ListDevice;
	CExtNumEdit m_EditPresetting;
	CButton m_CheckPresetting;
	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboDevice;
	CExtNumEditComboBox m_ComboFamily;
	
	CTADatabase *m_pTADB;				// Database containing TA products and pipes
	CTADatastruct *m_pTADS;				// Data structure constituting the document
	CString m_SelType;					// Selected type
	CString m_SelThingID;				// ID of the selected device in the list control
	CString m_SelectedID;				// ID of the selected device when the dialog box is called
	CDB_TAProduct *m_pProd;
	double m_dPresetting;
	double m_dMinOpening;
	double m_dMaxOpening;

// Private methods.
private:
	// Fill the Valve type Comboex box
	// Type specifies the default selection. If NULL, the 
	// "** All ..." string is selected.
	void _FillComboexType( LPCTSTR ptcType = NULL );
	
	// Retrieve the product type ID that is selected in the type ComboEx.
	LPCTSTR _GetSelTypeID();
	
	// Retrieve the product family ID that is selected in the family Combo.
	LPCTSTR _GetSelFamilyID();
};