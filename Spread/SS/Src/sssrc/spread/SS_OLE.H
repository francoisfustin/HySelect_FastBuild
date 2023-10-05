/************
* SS_OLE.H
************/

#ifndef SS_OLE_H
#define SS_OLE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SS_OCX)
LPDISPATCH SS_GetExtendedControl(LPSS_BOOK lpBook);
#endif

#ifdef __cplusplus
}
#endif

#endif
