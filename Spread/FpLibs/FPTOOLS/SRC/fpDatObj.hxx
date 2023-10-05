/***********************************************************************
* FPDATOBJ.Hxx - header for XfpDataObject class override for
*				 COleControl::XDataObject 
*
* Copyright (C) 1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
// this file should be included inside the class declaration for your control.
// NOTE: FPMYDISP_CTRL_NAME must be declared BEFORE including this file!!!
#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" before including fpprpbag.h ***" ) 
#endif

public:
// Following creates member variable:  XfpDataObject m_xfpDataObject;
	BEGIN_INTERFACE_PART(fpDataObject, IDataObject)
		INIT_INTERFACE_PART(FPMYDISP_CTRL_NAME, fpDataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, int);
		STDMETHOD(EnumFormatEtc)(ULONG, LPENUMFORMATETC FAR*);
		STDMETHOD(DAdvise)(struct tagFORMATETC *,ULONG,struct IAdviseSink *,ULONG *);
		STDMETHOD(DUnadvise)(ULONG);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
    END_INTERFACE_PART(fpDataObject)
