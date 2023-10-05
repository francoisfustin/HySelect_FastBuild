// PPGeneral.h : Declaration of the CPPGeneral

#ifndef __PPGENERAL_H_
#define __PPGENERAL_H_

#include "resource.h"       // main symbols
#include "fpatlpp.h"

EXTERN_C const CLSID CLSID_PPGeneral;

/////////////////////////////////////////////////////////////////////////////
// CPPGeneral
class ATL_NO_VTABLE CPPGeneral :
	public CfpAtlPropPage<CPPGeneral, &CLSID_PPGeneral>
{
public:
	CPPGeneral();

	enum {IDD = IDD_PPGENERAL};

DECLARE_REGISTRY_RESOURCEID(IDR_PPGENERAL)

BEGIN_COM_MAP(CPPGeneral) 
	COM_INTERFACE_ENTRY_IMPL(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CPPGeneral)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_HELP, OnWmHelp)
	COMMAND_HANDLER(IDC_ALLOWUSERZOOM, BN_CLICKED, EnableApply)
	COMMAND_HANDLER(IDC_ENABLED, BN_CLICKED, EnableApply)
	COMMAND_HANDLER(IDC_BORDERSTYLE, CBN_SELCHANGE, EnableApply)
	COMMAND_HANDLER(IDC_MOUSEPOINTER, CBN_SELCHANGE, EnableApply)
	COMMAND_HANDLER(IDC_SCROLLBARH, CBN_SELCHANGE, EnableApply)
	COMMAND_HANDLER(IDC_SCROLLBARV, CBN_SELCHANGE, EnableApply)
	COMMAND_HANDLER(IDC_SCROLLINCH, EN_CHANGE, EnableApply)
	COMMAND_HANDLER(IDC_SCROLLINCV, EN_CHANGE, EnableApply)
	CHAIN_MSG_MAP(IPropertyPageImpl<CPPGeneral>)
END_MSG_MAP()

	STDMETHOD(Apply)(void);
	void InitControls();
	void LoadComboStrings(void);
};

#endif //__PPGENERAL_H_
