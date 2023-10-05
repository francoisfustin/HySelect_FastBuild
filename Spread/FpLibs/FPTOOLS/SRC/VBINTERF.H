#ifndef VBINTERF_H
#define VBINTERF_H

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// IVBGetControl
//------------------------------------------------------------------------------------------------------------------------------------------------------
// This interface lives on the X-Object
//------------------------------------------------------------------------------------------------------------------------------------------------------

// Constants for dwWhich parameter: 
#define GC_WCH_SIBLING      0x00000001L
#define GC_WCH_CONTAINER    0x00000002L      // no FONLYAFTER/BEFORE
#define GC_WCH_CONTAINED    0x00000003L      // no FONLYAFTER/BEFORE
#define GC_WCH_ALL          0x00000004L    
#define GC_WCH_FREVERSEDIR  0x08000000L      // OR’d with others
#define GC_WCH_FONLYAFTER   0x10000000L      // OR’d with others
#define GC_WCH_FONLYBEFORE  0x20000000L      // OR’d with others
#define GC_WCH_FSELECTED    0x40000000L      //  OR’d with others 
#define GC_WCH_FEXTENDEROBJ 0x80000000L      // OR’d with others 

DECLARE_INTERFACE_(IVBGetControl, IUnknown)
 {
    // *** IUnknown methods ****
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IVBGetControl methods ****
    STDMETHOD (EnumControls) (THIS_ DWORD dwOleContF, DWORD dwWhich, LPENUMUNKNOWN FAR *ppenumUnk) PURE;
 };

//-------------------------------------------------------------------------------------------------------------------------------------------------
//IGetOleObject
//-------------------------------------------------------------------------------------------------------------------------------------------------
// This interface lives on the Extender Object (X-Object / hctl)
//------------------------------------------------------------------------------------------------------------------------------------------------
DECLARE_INTERFACE_(IGetOleObject, IUnknown)
  {
    // *** IUnknown methods ****
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,  AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IGetOleObject methods ****
    STDMETHOD(GetOleObject) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
  };

//-----------------------------------------------------------------------------------------------------------------------------------------------
// IVBDDE
//----------------------------------------------------------------------------------------------------------------------------------------------

typedef UINT ERR;

//----------------------------------------------------------------------------------------------------------------------------------------------
// Data xfer structure for VBM_LINKGETDATA or VBM_LINKSETDATA
//----------------------------------------------------------------------------------------------------------------------------------------------
typedef struct tagVBLINKDATA 
  {
    WORD     wReserved;
    DWORD    cb;           // Size of data
    HANDLE   hData;        // Handle to data
    DWORD    dwReserved; 
  } VBLINKDATA;

typedef VBLINKDATA FAR *LPLINKDATA;

//----------------------------------------------------------------------------------------------------------------------------------------------
// Link modes
//----------------------------------------------------------------------------------------------------------------------------------------------
#define LINKMODENONE                0
#define LINKMODEAUTO                1
#define LINKMODEMANUAL              2   
#define LINKMODENOTIFY              3

#define LINKMODESERVERPOKE          1
#define LINKMODESERVERNOPOKE        2

#define VBDDEDATA VBLINKDATA
#define LPDDEDATA LPLINKDATA

//------------------------------------------------------------------------------------------------------------------------------------------------
// Misc DDE defines
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_EXEC                256
#define MAXLINKITEMNAME         256

#define ACCEPTSDATAFORMAT       1
#define SUPPLIESDATAFORMAT      2

#define LINKSRCASK              0
#define LINKSRCTELL             1

#define LINK_DATA_OK            0
#define LINK_DATA_OOM           1
#define LINK_DATA_FORMATBAD     2
#define LINK_DATA_SETFAILED     3

/* Definition of interface: IVBDDESite  */
#undef INTERFACE
#define INTERFACE IVBDDESite
DECLARE_INTERFACE_(IVBDDESite, IUnknown)
  {
    /* IUnknown methods */
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    //(CLIENT) pass in a string and we will add the index
    STDMETHOD_(VOID, VBLinkMakeItemName) (THIS_ char FAR * lpszBuff) PURE;
    //(SERVER) notifies the server that the data exposed by this control
    // has changed.
    STDMETHOD_(ERR, VBLinkPostAdvise) (THIS) PURE;
    // (CLIENT) design time only. default implementation of LinkPasteLinkOk
    STDMETHOD_(BOOL, VBPasteLinkOk) (THIS_ HANDLE FAR * phTriplet) PURE;
    //(CLIENT) default inplementation of GetItemName
    STDMETHOD_(ERR, GetItemNameDefault) (THIS_ int ls, char FAR *lpszlinkitem) PURE;
  };
 
/* Definition of interface: IVBDDE  */
#undef INTERFACE
#define INTERFACE IVBDDE
DECLARE_INTERFACE_(IVBDDE, IUnknown)
  {
    /* IUnknown methods */
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    //(BOTH)
    STDMETHOD_(LRESULT, LinkEnumFormats) (THIS_ UINT df, UINT i) PURE;
    //both?
    STDMETHOD_(LRESULT, LinkGetData) (THIS_ int cf, LPDDEDATA lpddedata) PURE;
    STDMETHOD_(LRESULT, LinkSetData) (THIS_ int cf, LPDDEDATA lpddedata) PURE;
    STDMETHOD_(LRESULT, LinkGetItemName) (THIS_ int ls, char FAR *lpszlinkitem) PURE;
    //(CLIENT)
    STDMETHOD_(BOOL, QPasteLinkOk)  (THIS) PURE;
  };

#undef INTERFACE
#define INTERFACE IVBFormat
DECLARE_INTERFACE_(IVBFormat, IUnknown)
  {
    /* IUnknown methods */
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef) (THIS) PURE;
    STDMETHOD_(unsigned long, Release) (THIS) PURE;
    STDMETHOD(Format) (VARIANT FAR *vData, BSTR bstrFormat, LPVOID lpBuffer,
                       USHORT cB, LONG lcid, SHORT sFirstDayOfWeek,
                       USHORT sFirstWeekOfYear, USHORT * rcb) PURE;
  };

#undef INTERFACE
#define INTERFACE IVBFiles
DECLARE_INTERFACE_(IVBFiles, IUnknown)
  {
    /* IUnknown methods */
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef) (THIS) PURE;
    STDMETHOD_(unsigned long, Release) (THIS) PURE;

    STDMETHOD(Read)     (USHORT usFilesNo, LPVOID lpBuffer, UINT cb) PURE;
    STDMETHOD(Write)    (USHORT usFilesNo, LPVOID lpBuffer, UINT cb) PURE;
    STDMETHOD(Seek)     (USHORT usFileNo, ULONG lOffset, ULONG *plPos) PURE;
    STDMETHOD(SeekRel)  (USHORT usFileNo, ULONG lOffset, ULONG *plPos) PURE;
  };


#undef INTERFACE
#define INTERFACE IGetVBAObject
DECLARE_INTERFACE_(IGetVBAObject, IUnknown)
  {
    /* IUnknown methods */
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef) (THIS) PURE;
    STDMETHOD_(unsigned long, Release) (THIS) PURE;

    STDMETHOD(GetObject) (THIS_ REFIID riid, void FAR* FAR* ppvObj, DWORD dwReserved) PURE;
  };

#endif //VBINTERF_H
