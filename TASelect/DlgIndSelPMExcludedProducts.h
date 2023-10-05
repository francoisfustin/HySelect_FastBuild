#pragma once


#include "afxwin.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "RViewSSelPM.h"

class CDlgIndSelPMExcludedProducts : public CDialogExt
{
public:
	enum { IDD = IDD_DLGINDSELPMEXCLUDEDPRODUCTS };

	CDlgIndSelPMExcludedProducts( CSelectPMList *pclSelectPMList, CSelectPMList::ProductType eProductType, CRViewSSelPM *pRViewSSelPM );
	virtual ~CDlgIndSelPMExcludedProducts();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedClose();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnGetMinMaxInfo( MINMAXINFO* lpMMI );

	// Spread DLL message handlers.
	afx_msg LRESULT TextTipFetch( WPARAM wParam, LPARAM lParam );

// Private methods.
private:
	void _InitializeSSheet( void );
	void _ResizeColumns( void );
	void _UpdateLayout( int cx, int cy );

// Private variables.
private:
	enum SheetDescription
	{
		SD_ExcludedProduct = 1
	};
	
	CButton m_ButtonClose;
	CSelectPMList *m_pclSelectPMList;
	CSelectPMList::ProductType m_eProductType;
	CViewDescription m_ViewDescription;
	CSheetDescription *m_pclSheetDescription;
	CSSheet *m_pclSheet;
	std::map<int, int> m_mapDefaultColWidth;
	int m_iTotalColWidth;
};
