#pragma once


#include "DlgCtrlPropPage.h"
#include "afxwin.h"
#include "ExtComboBox.h"
#include "XGroupBox.h"

class CDlgTPPageHC : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPPageHC )

public:
	typedef enum RemarkDN
	{
		DN50,
		DN65,
		LastDN
	};
	
	typedef enum RemarkType
	{
		DN,
		CV,
		LastType
	};
	
	typedef enum AboveOrBelow
	{
		Below65,
		Above50
	};

	enum { IDD = IDD_TABDIALOGTECHHYDROCALC };

	CDlgTPPageHC( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool bResetToDefault = false );
	virtual void Save( CString strSectionName );
	// HYS-1221 : Update HM products preferences combo box
	virtual void UpdateComboWithDeleted();

	ProductSubCategory GetProductSubCategory() { return m_eProductSubCategory; }
	void SetProductSubCategory( ProductSubCategory eProductSubCategory ) { m_eProductSubCategory = eProductSubCategory; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnCbnSelChangeTypeBelow65();
	afx_msg void OnCbnSelChangeTypeAbove50();
	afx_msg void OnCbnSelChangeFamilyBelow65();
	afx_msg void OnCbnSelChangeFamilyAbove50();
	afx_msg void OnCbnSelChangeBodyMaterialBelow65();
	afx_msg void OnCbnSelChangeBodyMaterialAbove50();
	afx_msg void OnCbnSelChangeConnectBelow65();
	afx_msg void OnCbnSelChangeConnectAbove50();
	afx_msg void OnCbnSelChangeVersionBelow65();
	afx_msg void OnCbnSelChangeVersionAbove50();
	afx_msg void OnCbnSelChangePNBelow65();
	afx_msg void OnCbnSelChangePNAbove50();

	void ShowInfos( RemarkDN eDN, RemarkType eType, bool bShow, CString strText = _T("") );
	void FillComboBoxType( AboveOrBelow eAboveOrBelow );
	void FillComboBoxFamily( AboveOrBelow eAboveOrBelow );
	void FillComboBoxBodyMaterial( AboveOrBelow eAboveOrBelow );
	void FillComboBoxConnect( AboveOrBelow eAboveOrBelow );
	void FillComboBoxVersion( AboveOrBelow eAboveOrBelow );
	void FillComboBoxPN( AboveOrBelow eAboveOrBelow );
	void FillComboBox( CRankEx *pListEx, CExtNumEditComboBox *pCCombo, int iCount, CString *pCstr = NULL );
	void WarningMsgCV( RemarkDN eDN );
	void WarningMsgDN( RemarkDN eDN );

// Private variables.
private:
	CTADatastruct *m_pTADS;
	
	ProductSubCategory m_eProductSubCategory;
	CArray<CTADatabase::CvTargetTab, CTADatabase::CvTargetTab> m_arTargetTab;

	CExtNumEditComboBox m_ComboTypeBelow65;
	CExtNumEditComboBox m_ComboTypeAbove50;
	CExtNumEditComboBox m_ComboFamilyBelow65;
	CExtNumEditComboBox m_ComboFamilyAbove50;
	CExtNumEditComboBox m_ComboBodyMaterialBelow65;
	CExtNumEditComboBox m_ComboBodyMaterialAbove50;
	CExtNumEditComboBox m_ComboConnextBelow65;
	CExtNumEditComboBox m_ComboConnectAbove50;
	CExtNumEditComboBox m_ComboVersionBelow65;
	CExtNumEditComboBox m_ComboVersionAbove50;
	CExtNumEditComboBox m_ComboPNBelow65;
	CExtNumEditComboBox m_ComboPNAbove50;

	CStatic m_InfoType1;
	CStatic m_InfoTypeNbr1;
	CStatic m_Info1;
	CStatic m_InfoNbr1;
	CStatic m_InfoType2;
	CStatic m_InfoTypeNbr2;
	CStatic m_Info2;
	CStatic m_InfoNbr2;
	
	CImageList m_ImageList;	
	
	CEdit m_EditBoxText1;
	CEdit m_EditBoxText2;
	CEdit m_EditBoxText3;

	CXGroupBox m_GroupBelow50;
	CXGroupBox m_GroupAbove65;

	bool m_bRemarkCV50;
	bool m_bRemarkDN50;
	bool m_bRemarkCV65;
	bool m_bRemarkDN65;
	CString m_strRemarkCV50;
	CString m_strRemarkDN50;
	CString m_strRemarkCV65;
	CString m_strRemarkDN65;

	CString m_strTypeBelow65;
	CString m_strTypeAbove50;
	CString m_strFamilyBelow65;
	CString m_strFamilyAbove50;
	CString m_strBodyMaterialBelow65;
	CString m_strBodyMaterialAbove50;
	CString m_strConnectBelow65;
	CString m_strConnectAbove50;
	CString m_strVersionBelow65;
	CString m_strVersionAbove50;
	CString m_strPNBelow65;
	CString m_strPNAbove50;

	bool m_bShowPayAttention;
};
