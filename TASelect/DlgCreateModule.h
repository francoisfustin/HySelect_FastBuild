#pragma once


#include "CbiRestrEdit.h"
#include "ExtComboBox.h"
#include "HydroMod.h"

class CDlgCreateModule : public CDialogEx
{
public:
	enum { IDD = IDD_DLGCREATEMODULE };

	enum DialogMode
	{
		CreateModule,
		EditModule,
		CreateValve,
		EditValve
	};

	class INotification
	{
	public:
		virtual void OnChangeLocatedIn( CDlgCreateModule* pDialog ) {}
	};

	CDlgCreateModule( CWnd* pParent = NULL, INotification *pclNotification = NULL );	 // Standard constructor.
	CDlgCreateModule( CView* pView, INotification *pclNotification = NULL );			 // Modeless constructor.

	//	Display :   DlgMode			to select between creation, edition, valve and module.
	//				strCaption		dialog title.
	//				strReference	Reference stored in the TADS selected valve.
	//				pPHM			CDS_Hydromod* pointer on the parent module, null when the parent module is the ROOT_MODULE.
	//				iIndex			current valve index in the current module.
	//				iMaxIndex		maximum index for this valve.
	//				SelTADSThingID	current selected valve.
	int Display( DialogMode eDlgMode, CString strCaption, CString strReference, CDS_HydroMod *pPHM, int iIndex, int iMaxIndex, double dFlow, 
				CString strSelTADSThingID, enum_VDescriptionType eVDescrType, double dKvCv, double dPresetting = -1.0 );

	//	pHM is the CDS_HydroMod*
	int Display( DialogMode eDlgMode, CString strCaption, CDS_HydroMod *pHM, int iMaxIndex );

	CDS_HydroMod *GetParentModule() { return m_pPHM; }
	int GetPosition() { return m_iPosition; }
	CString GetReference() { return m_strReference; }
	void SetDescription( CString str ) { m_strDescription = str; }
	CString GetDescription() { return m_strDescription; }
	double GetFlow() { return m_dFlow; }
	double GetKvCv() { return m_dKvCv; }
	double GetPresetting() { return m_dSelPresetting; }
	CString GetSelTADBID() { return m_strSelTADBID; }
	void SetSelTADBID( CString ID ) { m_strSelTADBID = ID; }
	bool IsValveChanged() { return m_bValveChanged; }
	enum_VDescriptionType GetVDescrType() { return m_eVDescrType; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnMove( int x, int y );
	afx_msg void OnCbnSelChangeLocatedIn();
	afx_msg void OnDeltaSpinPosition( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnCheckPartnerValve();
	afx_msg void OnEnChangeFlow();
	afx_msg void OnButtonKvCvInput();
	afx_msg void OnButtonDirectSelection();

// Private methods.
private:
	void _ChangeReference( void );
	bool _CheckEditQ( void );
	void _FillComboParent( CDS_HydroMod *pHM );

// Private variables.
private:
	CView *m_pView;
	CExtNumEditComboBox m_ComboLocatedIn;
	CEdit m_EditPosition;
	CSpinButtonCtrl m_SpinPosition;
	CCbiRestrEdit m_clEditReference;
	CEdit m_EditDescription;
	CButton m_CheckPartnerValve;
	CEdit m_EditFlow;
	CEdit m_EditValve;
	CButton m_ButtonKvCvInput;
	CButton m_ButtonDirectSel;

	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CUnitDatabase *m_pUnitDB;
	DialogMode m_eDlgMode;
	CString m_strCaption;
	bool m_bInitialized;
	int m_iPosition;
	int m_iMax;
	CString m_strReference;
	CString m_strDescription;
	double m_dFlow;
	double m_dSelPresetting;
	double m_dKvCv;
	bool m_bValveChanged;
	CString m_strSelTADBID;
	CString m_strSelTADBCategory;
	CString m_strCBIType;
	CString m_strCBISize;
	CBitmap m_Bitmap;
	CDS_HydroMod *m_pPHM;
	enum_VDescriptionType m_eVDescrType;
	INotification *m_pclNotification;
};