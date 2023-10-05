// NOTE: FPMYDISP_CTRL_NAME must be declared BEFORE including this file!!!
#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" before including fpprpbag.h ***" ) 
#endif

// Following creates member variable:  XfpPersistPropertyBag m_xfpPersistPropertyBag;
public:

// IPersistPropertyBag
   BEGIN_INTERFACE_PART(fpPersistPropertyBag, IPersistPropertyBag)
   	   INIT_INTERFACE_PART(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)
	   STDMETHOD(GetClassID)(LPCLSID);
	   STDMETHOD(InitNew)();
	   STDMETHOD(Load)(LPPROPERTYBAG, LPERRORLOG);
	   STDMETHOD(Save)(LPPROPERTYBAG, BOOL, BOOL);
   END_INTERFACE_PART(fpPersistPropertyBag)

