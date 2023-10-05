/***********************************************************************
* FPDBSIMP.H
*
* Copyright (C) 1991-1997 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
*
************************************************************************
*
* Declaration of the CfpOleSimpleBound class.
*
* FarPoint's implementation of OLE's "Simple" DB Binding
* for OLE controls.
*
* This class is intented to be a member variable (public) of
* an OCX control (subclassed from COleControl).
***********************************************************************/
#ifndef FPDBSIMP_H
#define FPDBSIMP_H

//-------------------------------------------------------------------------
// "Public" Class - CfpOleBinding
//-------------------------------------------------------------------------
class CfpOleSimpleBound : public CObject
{
//-------------------------------------------------------------------------
// PUBLIC MEMBER FUNCTIONS
//-------------------------------------------------------------------------
public:
// Constructor
	CfpOleSimpleBound();
// Destructor
	~CfpOleSimpleBound();

// Set-up of Environment
    void SetOwner( LPVOID lpOleControl);
    void SetBoundValDispid( DISPID dispid);

    // NOTE: lpfuncDataMsg is ptr to functions of type:
    //       extern "C" void DataMsg( LPVOID lpObject, UINT uMsg,
    //                                WPARAM wParam, LPARAM lParam);
    //       When the function is invoked, "lpObject" is ptr to "Owner".
    void SetDataMsgFunc( LPVOID lpfuncDataMsg);

// Interface to Data functions (LPVOID -> LPDATAACCESS)
    short DataCall(LPVOID lpvda, USHORT msg, USHORT action, 
      LPVOID lpvUserData = NULL);

#ifndef NO_MFC // MFC
#define FP_PROPRETTYPE  BOOL    // Get/Set functions return BOOL result
#define FP_PROPRETVAL(mfcval, nomfcval)  mfcval
#else          // NO_MFC
#define FP_PROPRETTYPE  HRESULT // Get/Set functions return HRESULT result
#define FP_PROPRETVAL(mfcval, nomfcval)  nomfcval
#endif

    // "Interface" for control's Get/Set property functions.
    FP_PROPRETTYPE fpGetBoundVal(LPVARIANT lpVar);
    FP_PROPRETTYPE fpGetBoundVal(BSTR FAR *lpbstr);  // for BSTR only

    FP_PROPRETTYPE fpSetBoundVal(LPVARIANT lpVar);
    FP_PROPRETTYPE fpSetBoundVal(LPCTSTR lpctstr);   // for BSTR only

//-------------------------------------------------------------------------
// PUBLIC MEMBER VARIABLES
//-------------------------------------------------------------------------
public:

//-------------------------------------------------------------------------    
// PROTECTED MEMBER FUNCTIONS...
//-------------------------------------------------------------------------    
protected:

    short ProcessData_Get( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);
    short ProcessData_Set( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);

    USHORT GetVBDataType(void);
    BOOL GetDataValue( BOOL fGetLenOnly, LPVOID lpvda);
    BOOL SetDataValue(LPVOID lpvda);

// Functions related to Error codes when Updating the record source.
    BOOL fpQueryError(void);
    void fpResetError(LPVOID lpvUserData = NULL);
    void fpSetError(LONG lErrorCode, LONG lRcFromCursorAPI, 
      LPVOID lpvQueryInterfacePtr);
    LONG fpInvokeError(void);
#define FPDB_ERR_BEGINUPDATE_ADD       5000
#define FPDB_ERR_BEGINUPDATE_UPDATE    5001   
#define FPDB_ERR_CANTSETAUTOINCR       5002   
#define FPDB_ERR_CONVERTSTRINGTOTYPE   5003   
#define FPDB_ERR_DELETE                5004   
#define FPDB_ERR_FETCHCURRENTROW       5005   
#define FPDB_ERR_GETNEXTROW            5006   
#define FPDB_ERR_LBLOCKALLOC           5007   
#define FPDB_ERR_MEMLOCK_DATABUFFER    5008   
#define FPDB_ERR_NOTIMPLEMENTED        5009   
#define FPDB_ERR_QINTF_UPDATECURSOR    5010   
#define FPDB_ERR_SETCOLUMN             5011  
#define FPDB_ERR_UPDATE                5012  
#define FPDB_ERR_UPDATEINPROGRESS      5013  
#define FPDB_ERR_MOVETO_NEWROW         5014
    
//-------------------------------------------------------------------------    
// PROTECTED MEMBER VARIABLES...
//-------------------------------------------------------------------------    
protected:
// Dependent on environment (OLE Control)
#ifdef SS40
    VOID FAR *m_lpOwner;
#elif !defined(NO_MFC)
    COleControl FAR *m_lpOwner;            // Pointer to OCX class (owner)    
#else
    CfpAtlBase  FAR *m_lpOwner;            // Pointer to OCX class (owner)    
#endif

// Function ptr.  "lpObject" is a ptr to Owner object
    LRESULT (FAR *m_lpfuncDataMsg)( LPVOID lpObject,
                                    UINT uMsg, WPARAM wParam, LPARAM lParam);
    DISPID      m_dispid;
    ULONG       m_ulCtlFlags;       // Used to simulate VBSetControlFlags

    BOOL        m_fValSet;          // whether the value was set
    VARIANT     m_varValSet;        // the last value from fpSetBoundVal().

    BOOL         m_fCurrGet;        // set to TRUE if control changed the value.
    VARIANT FAR *m_lpvarCurrGet;    // save temporarly

    BOOL        m_fOnlyBSTR;        // whether value can only be a BSTR

// member vars related to Error codes when Updating the record source.
    int      m_iErrorCt;
    struct 
    {
      LONG lErrorCode;
      LONG lExtErrorCode;
      LPVOID lpvUserData;
    } m_ErrorInfo;

};

#endif  // ifndef FPDBSIMP_H
