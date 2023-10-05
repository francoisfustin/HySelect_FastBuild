// DLGRESOLCBIPLANTMODIF.cpp : implementation file
//

#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include "TASelect.h"

#include "global.h"
#include "utilities.h"
#include "Hydromod.h"
#include "DlgResolCBIPlantModif.h"


// CDlgResolCBIPlantModif dialog

//IMPLEMENT_DYNAMIC(CDlgResolCBIPlantModif, CDlgListBox)
CDlgResolCBIPlantModif::CDlgResolCBIPlantModif(CWnd* pParent /*=NULL*/)
	: CDlgListBox(pParent)
{
	m_papHM = new CRank(false);
}

CDlgResolCBIPlantModif::~CDlgResolCBIPlantModif()
{
	if (m_papHM)
	{
		m_papHM->PurgeAll();
		delete (m_papHM);
		m_papHM = NULL;
	}
}

void CDlgResolCBIPlantModif::Display(bool &bOK)
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	SetColumn(4);
	AddHeaderStr (ecModification, IDS_DLGRESOLCBIPLANTMODIF_HEADERDETMODIF);
	AddHeaderStr (ecValveName, IDS_DLGRESOLCBIPLANTMODIF_HEADERVALVENAME);
	AddHeaderStr (ecValve, IDS_DLGRESOLCBIPLANTMODIF_HEADERVALVE);
	AddHeaderStr (ecPosition, IDS_DLGRESOLCBIPLANTMODIF_HEADERPOS);
	// Fill DlgCannotSend with problem messages
	m_strProblemPos[epMoved] = AddStrToList(1,IDS_DLGRESOLCBIPLANTMODIF_HMMOVEDDESC);
	m_strProblemPos[epDeleted] = AddStrToList(1,IDS_DLGRESOLCBIPLANTMODIF_HMDELETEDDESC,2);
	m_strProblemPos[epAddedOK] = AddStrToList(1,IDS_DLGRESOLCBIPLANTMODIF_HMADDEDOKDESC);
	m_strProblemPos[epAddedNOK] = AddStrToList(0,IDS_DLGRESOLCBIPLANTMODIF_HMADDEDNOKDESC,1);
	m_strProblemPos[epValveModifiedOK] = AddStrToList(1,IDS_DLGRESOLCBIPLANTMODIF_HMMODIFIEDOKDESC);
	m_strProblemPos[epValveInKVmode] = AddStrToList(0,IDS_DLGRESOLCBIPLANTMODIF_VALVEINKVMODEDESC,2);
	bOK = true;
	CString	str;
	LPARAM lparam=NULL;
	for (BOOL cont=m_papHM->GetFirst(str,lparam); cont; cont=m_papHM->GetNext(str,lparam))
	{
		CDS_HydroMod *pHM=(CDS_HydroMod *)lparam;	
		ASSERT(pHM);
		// Test existance of a primary BV or a TA CV
		if (!(pHM->IsBvExist() || (pHM->IsCvExist() && pHM->GetpCV()->IsTaCV()))) continue;
		switch (pHM->GetTreatment())
		{
			case ett_Added:
				{
					if (pHM->GetVDescrType() == edt_KvCv)
					{
						str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_VALVEINKVMODE);
						AddStr(ecModification,str);
						AddStr(ecValveName,_T(""));
						if (!pUnitDB->GetDefaultUnitIndex(_C_KVCVCOEFF))
							str=TASApp.LoadLocalizedString(IDS_KVS);
						else
							str=TASApp.LoadLocalizedString(IDS_CV);
						str+=CString(_T("="))+WriteCUDouble(_C_KVCVCOEFF,pHM->GetKvCv());
						AddStr(ecValve,str);
						str.Format(_T("%d"),pHM->GetPos());
						AddStr(ecPosition,str);
						AddLine((LPARAM)m_strProblemPos[epValveInKVmode]);
						bOK = false;
					}
					else
					{
						if (pHM->SolvePartDef())
						{
							// Added OK
							str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_HMADDED);
							AddStr(ecModification,str);
							AddStr(ecValveName,pHM->GetHMName());
							str = pHM->GetCBIType()+ CString(_T(" "))+ CString(pHM->GetCBISize());
							AddStr(ecValve,str);
							CString strpos;
							strpos.Format(_T("%d"),pHM->GetPos());
							AddStr(ecPosition,strpos);
							CString strline;
							CString strparent;
							strparent=((CDS_HydroMod*)pHM->GetIDPtr().PP)->GetHMName();
							FormatString(strline,IDS_DLGRESOLCBIPLANTMODIF_HMADDEDOKDESC,pHM->GetHMName(),strparent,strpos);
							POSITION pos = AddStrToList(1,strline);
							AddLine((LPARAM)pos);
							bOK = true;
						}
						else
						{	// Added NOK
							str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_HMADDED);
							AddStr(ecModification,str);
							AddStr(ecValveName,pHM->GetHMName());
							str = pHM->GetCBIType()+ CString(_T(" "))+ CString(pHM->GetCBISize());
							AddStr(ecValve,str);
							AddStr(ecPosition,_T(""));
							AddLine((LPARAM)m_strProblemPos[epAddedNOK]);
							bOK = false;
						}
					}
				}
			break;
			case ett_ValveModified:
				{
					if (pHM->GetVDescrType() == edt_KvCv)
					{
						str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_VALVEINKVMODE);
						AddStr(ecModification,str);
						AddStr(ecValveName,_T(""));
						if (!pUnitDB->GetDefaultUnitIndex(_C_KVCVCOEFF))
							str=TASApp.LoadLocalizedString(IDS_KVS);
						else
							str=TASApp.LoadLocalizedString(IDS_CV);
						str+=CString(_T("="))+WriteCUDouble(_C_KVCVCOEFF,pHM->GetKvCv());
						AddStr(ecValve,str);
						str.Format(_T("%d"),pHM->GetPos());
						AddStr(ecPosition,str);
						AddLine((LPARAM)m_strProblemPos[epValveInKVmode]);
						bOK = false;
					}
					else
					{
						if (pHM->SolvePartDef())
						{
							str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_HMVALVEMODIFIED);
							AddStr(ecModification,str);
							AddStr(ecValveName,pHM->GetHMName());
							str = pHM->GetCBIType()+ CString(_T(" "))+ CString(pHM->GetCBISize());
							AddStr(ecValve,str);
							str.Format(_T("%d"),pHM->GetPos());
							AddStr(ecPosition,str);
							CString strold,strcbi,strline;
							CDB_TAProduct *pTAp = (CDB_TAProduct *)pHM->GetTreatmentLparam();
							if (pTAp)
							{
								strold=pTAp->GetCBIType()+ CString(_T(" "))+ CString(pTAp->GetCBISize());
								strcbi=pHM->GetCBIType()+ CString(_T(" "))+ CString(pHM->GetCBISize());
								CDB_TAProduct *pTAProd = dynamic_cast<CDB_TAProduct *>(pHM->GetCBIValveIDPtr().MP);
								if (pTAProd)
									FormatString(strline,IDS_DLGRESOLCBIPLANTMODIF_HMMODIFIEDOKDESC,strold,strcbi,pTAProd->GetName());
								POSITION pos = AddStrToList(1,strline);
								AddLine((LPARAM)pos);
								bOK = true;
							}
							else
								bOK = false;
						}
						else
						{	// Modified NOK
							str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_HMVALVEMODIFIED);
							AddStr(ecModification,str);
							AddStr(ecValveName,pHM->GetHMName());
							str = pHM->GetCBIType()+ CString(_T(" "))+ CString(pHM->GetCBISize());
							AddStr(ecValve,str);
							AddStr(ecPosition,_T(""));
							AddLine((LPARAM)m_strProblemPos[epAddedNOK]);
							bOK = false;
						}
					}
				}
			break;
			case ett_Delete:
				{
					str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_HMDELETED);
					AddStr(ecModification,str);
					AddStr(ecValveName,pHM->GetHMName());
					str = pHM->GetCBIType()+ CString(_T(" "))+ CString(pHM->GetCBISize());
					AddStr(ecValve,str);
					str.Format(_T("%d"),pHM->GetPos());
					AddStr(ecPosition,str);
					AddLine((LPARAM)m_strProblemPos[epDeleted]);
					bOK = true;
				}
			break;
			case ett_Moved:
				{
					str=TASApp.LoadLocalizedString(IDS_DLGRESOLCBIPLANTMODIF_HMMOVED);
					AddStr(ecModification,str);
					AddStr(ecValveName,pHM->GetHMName());
					str = pHM->GetCBIType()+ CString(_T(" "))+ CString(pHM->GetCBISize());
					AddStr(ecValve,str);
					str.Format(_T("%d"),pHM->GetPos());
					AddStr(ecPosition,str);
					CString oldpos;
					oldpos.Format (_T("%d"),pHM->GetTreatmentLparam());
					CString line;
					FormatString(line,IDS_DLGRESOLCBIPLANTMODIF_HMMOVEDDESC,pHM->GetHMName(),oldpos,str);
					POSITION pos = AddStrToList(1,line);
					AddLine((LPARAM)pos);
					bOK = true;
				}
			break;
			default:
			break;
		}
	}
	__super::Display(IDS_DLGRESOLCBIPLANTMODIF_CAPTION, IDS_DLGRESOLCBIPLANTMODIF_STATICTITLE, IDS_DLGRESOLCBIPLANTMODIF_STATICDESCRIPTION, IDB_CANNOTSENDSIGNS);
}

void CDlgResolCBIPlantModif::FillHMarray(CTable *pTab)
{
	if (!pTab) return;
	for (IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext())
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;
		if (pHM->GetTreatment() != ett_Kept && pHM->GetTreatment() != ett_None)
			m_papHM->Add(pHM->GetHMName(),-1*(pHM->GetLevel()*1000-pHM->GetPos()),(LPARAM) pHM);
		if (pHM->IsaModule())
			FillHMarray(pHM);
	}
}

// Initialize some member variables, return true if DialogBox must be displayed
bool CDlgResolCBIPlantModif::Init( CString TabID )
{
	CTable *pTab = (CTable*)( TASApp.GetpTADS()->Get( (LPCTSTR)TabID ).MP );
	ASSERT( NULL != pTab );

	m_papHM->PurgeAll();
	FillHMarray( pTab );

	if( 0 == m_papHM->GetCount() )
	{
		return false;
	}
	
	return true;
}
