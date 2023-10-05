#pragma once


#include "TCImageList.h"
#include "MyToolTipCtrl.h"
#include "afxcmn.h"
#include "afxwin.h"

class CDlgImportHM : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgImportHM )

public:
	enum { IDD = IDD_DLGIMPORTHM };

	CDlgImportHM( CWnd *pParent = NULL );
	
	// HYS-1376 : Added for import unit test.
	// strFileNameToImport : file to import.
	// pArrayParam : Informations for the import process ( elements to import, parents, where to insert ... ).
	CDlgImportHM( CString strFileNameToImport, CStringArray *pArrayParam, int iArraySize, CWnd *pParent = NULL );
	virtual ~CDlgImportHM();

	// HYS-1376 : Add the items to be imported and import hydromod.
	void AddItemsUnitTests( HTREEITEM hItemParentSrc = NULL, HTREEITEM hItemParentDest = NULL );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnItemAdded( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT UpdateBtns( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg void OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTvnGetInfoTip( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnBnClickedOpenProject();
	afx_msg void OnBnClickedImportProject();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedExpandTree1();
	afx_msg void OnBnClickedCollapseTree1();
	afx_msg void OnBnClickedExpandTree2();
	afx_msg void OnBnClickedBtnCollapseTree2();

	// HYS-1359 : Returns now a booleen to know the status of import HM
	bool ImportHMRecursivelly( HTREEITEM hItem = NULL, CDS_HydroMod *pHMParent = NULL, CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined );
	
	// Return true if at least one HM has been imported in the m_TreeCurrProj.
	bool IsHMImported( HTREEITEM hItem = NULL );

	// HYS-1716: Check if some items ready to import have their secondary temperatures in error.
	// Imagine we have in source *A (injection circuit) and *A.1 that we drag & drop twice in this way in the destination:
	//
	//		*A
	//       |_ xx(*A) [1]
	//              |_ xx(*A.1) [1]
	//              |_ xx(*A) [2]
	//                      |_ xx(*A.1) [2]
	//
	// So, we drag & drop one time *A and its children *A.1 from the source as a child of *A in the destination -> [1].
	// And we also drag & drop *A and *A.1 now as child of xx(*A) [1] -> [2].
	// When running verification of temperatures, we take first the *A in the tree. For each item in the tree there 
	// is a structure ('m_UserData') that contains the 'bImported' variable that tells if the current item in the tree is an existing
	// item in the destination table ('bImported' = false) or if it's an item coming from the source ('bImported' = true).
	// Here *A is the one from the existing destination than we can directyl go to check its children. 
	// Because in our case xx(*A) [1] is an injection circuit we call the 'CTableHM::IsInjectionCircuitTemperatureErrorForPasteOperation' 
	// method to verify xx(*A) [1] and all of its children.
	// BUT ATTENTION: Physically (In the source table) xx(*A) [1] has only one child that is xx(*A.1) [1].
	// xx(*A) [2] is visible in the destination tree but is not really inserted in the table xx(*A) [1].
	// And the 'CTableHM::IsInjectionCircuitTemperatureErrorForPasteOperation' method we call the 'CTableHM:FillInjectionCircuitMap' method 
	// to fill a map with all the injection circuits that it contains. Here, there will be only xx(*A) [1] and not xx(*A) [2].
	// So, we need also to verify xx(*A) [2] and its children. And we will run the children of xx(*A) [1] (From the tree and not the source).
	// But, xx(*A.1) [1] has been already verified and we don't need to do it anymore. It's not really a problem because we are doing recursivity
	// and we pass always the HTREEITEM parent as argument. We can thus verify if the current child is the real child of its parent:
	//  -> If it's the case ('CDS_HydroMod' in 'm_UserData' of the HTREEITEM parent is the parent of the 'CDS_HydroMod' in the current HTREEITEM child)
	//     we don't need to verify temperatures but we must continue to go deeper if there are also children (Because we can have other modules and children
	// 	   dragged & dropped in this currrent HTREEITEM).
	//  -> If it's not the case, we need to verify temperatures and going deeper in the same way.
	bool IsSecondaryTemperatureErrors( HTREEITEM hItem = NULL, std::vector<CDS_HydroMod *> *pvecErrorList = NULL );

	// HYS-1716: Allow to retrieve the parent of an imported item with which we want to verify the secondary temperatures.
	// For the 'CTableHM::IsInjectionCircuitTemperatureErrorForPasteOperation' method we need each time parent with which we want to verify the
	// secondary temperatures. Imagine we are running the xx(*A) [2] item in the tree and we retrieve its parent that is thus xx(*A) [1].
	// Even if xx(*A) [1] is not the true parent in the source, if import is done, it will become the true parent. So we need to check with it.
	// We can have three cases here:
	//     1) xx(*A) [1] is imported from source and is already in error. We need to go higher to find the first imported parent that is not in 
	//        error or the first parent that is not imported.
	//     2) xx(*A) [1] is imported from source and not in error. We can take this one to verify.
	// 	   3) xx(*A) [1] is not imported, we can take this one to verify.
	// We also need a method that will flag source circuits in error or not.
	HTREEITEM GetParentNotInError( HTREEITEM hItem );

	// HYS-1716: Allow to set that these items are in error.
	// 'vecErrorList' is filled by the 'IsInjectionCircuitTemperatureErrorForPasteOperation' method.
	void SetTemperatureErrorFlag( std::vector<CDS_HydroMod *> *pvecErrorList );

// Protected variables.
protected:
	typedef struct _m_UserData
	{
		struct _m_UserData()
		{
			pHM = NULL; bImported = false; bDistPipes = false; bCircPipes = false; bBv = false; bBvByp = false; bDpC = false; bCv = false; bIsSecondaryTemperaturesError = false;
		};

		CDS_HydroMod *pHM;
		bool bImported;
		bool bDistPipes;
		bool bCircPipes;
		bool bBv;
		bool bBvByp;
		bool bDpC;
		bool bCv;
		bool bIsSecondaryTemperaturesError;
	}m_UserData;

	CHMTreeListCtrl	m_TreeProjToImport;
	CHMTreeListCtrl	m_TreeProjCurrent;
	CTCImageList m_TreeImageList;
	CButton m_BtnOpenProj;
	CButton m_BtnImport;
	CMFCButton m_BtnExpandTree1;
	CMFCButton m_BtnCollapseTree1;
	CMFCButton m_BtnExpandTree2;
	CMFCButton m_BtnCollapseTree2;
	CEdit m_EditInfo;
	CStatic m_StaticExtProj;
	CStatic m_StaticCurrProj;
	CTADatastruct *m_pDataStructToImport;
	ProjectType m_eCurrentProjectType;
	ProjectType m_eImportProjectType;
	CMyToolTipCtrl m_ToolTip;
	
	// HYS-1376.
	CString m_FileNameForUnitTests;		// Filename to import.
	CStringArray* m_pInputUnitTests;	// Input for import unit test.
	int m_iArraySize; // m_pInputUnitTests size.
};
