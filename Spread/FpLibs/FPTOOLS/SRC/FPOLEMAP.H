/*************************************************************************
 * FPOLEMAP.H  -  FarPoint's override of COleControl's Interface Map.
 *
 *   This is the work-around to the Blob problem between VC2.2 & VC4.0.
 *   The cause is the implementation of IPersistPropertyBag, so we
 *   override the interface map for COleControl and leave-out that 
 *   interface.
 *
 *   Include this file inside a .cpp file within the following section
 *   of code:
 *
 *     #define FPOLEMAP_CTRL_CLSNAME  CTabOCX  // Your control's class name
 *
 *     BEGIN_INTERFACE_MAP(CTabOCX, CWnd)      // use CWnd not COleControl 
 *     #include "fpolemap.h"
 *     END_INTERFACE_MAP() 
 *
 *
 *   NOTE: Before including the file, you must define FPOLEMAP_CTRL_CLSNAME
 *   as the name of your control class (ex. CTabOCX).
 *
 *   NOTE 2: You must declare the following in your class declaration (you
 *   may already have it declared):
 *
 *     DECLARE_INTERFACE_MAP()
 *
 ************************************************************************/
#if (!defined(NO_FPOLEMAP))

//BEGIN_INTERFACE_MAP(CTRL_CLS_FOR_FPOLEMAP, CWnd)  // use CWnd not COleControl 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IOleObject, OleObject) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IConnectionPointContainer, 
    ConnPtContainer) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IOleControl, OleControl) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IPersist, PersistStorage) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IPersistMemory, PersistMemory) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IPersistStreamInit, 
    PersistStreamInit) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IOleInPlaceObject, 
    OleInPlaceObject) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IOleInPlaceActiveObject, 
    OleInPlaceActiveObject) 
#ifndef FPMYDISP_CTRL_NAME    // if NOT overriding IDispatch, include it.
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IDispatch, Dispatch)
#endif   
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IOleCache, OleCache) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IViewObject, ViewObject) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IViewObject2, ViewObject) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IDataObject, DataObject) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_ISpecifyPropertyPages, 
    SpecifyPropertyPages) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IPerPropertyBrowsing, 
    PerPropertyBrowsing) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IProvideClassInfo, 
    ProvideClassInfo) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IProvideClassInfo2, 
    ProvideClassInfo) 
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IPersistStorage, PersistStorage) 
//END_INTERFACE_MAP() 

#endif
