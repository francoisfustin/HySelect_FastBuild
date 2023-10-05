#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "DlgListBox.h"

// CDlgResolCBIPlantModif dialog

class CDlgResolCBIPlantModif : public CDlgListBox
{
public:
	CDlgResolCBIPlantModif(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgResolCBIPlantModif();

// Dialog Data
	//enum { IDD = IDD_DLGRESOLCBIPLANTMODIF };

protected:

	enum eColumn
	{
		ecModification,
		ecValveName,
		ecValve,
		ecPosition
	};
	enum eProblem
	{
		epMoved=0,
		epDeleted=1,
		epAddedOK=2,
		epAddedNOK=3,
		epValveModifiedOK=4,
		epValveModifiedNOK=3,
		epValveInKVmode=5
	};
	CRank *m_papHM;
	POSITION m_strProblemPos[8];

	void FillHMarray(CTable *pTab);
public:
	void Display(bool &bOK);
	// Initialise some member variables, return true if DialogBox must be displayed
	bool Init(CString TabID=_T("PIPING_TAB"));
};
