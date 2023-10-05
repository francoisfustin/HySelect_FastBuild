/*********
* TBVB.H
*********/

#ifndef TBVB_H
#define TBVB_H

#define ERR_MEMORY               7
#define ERR_INVALID_PROPVAL      380
#define ERR_INVALID_PROPARRAYVAL 381

#define VA_LICENSE_FILE          "vbva.lic"

#define OFFSETIN(struc, field) ((BYTE)(long)(LPVOID)&(((struc *)0)->field))
   
#define DEFINEPROP(name, flags, enumlist, enumsize) \
static PROPINFO Property##name = {  \
   #name,                           \
   flags | PF_fGetMsg | PF_fDefVal | PF_fNoInitDef,   \
   0, 0, enumlist, NULL, enumsize   \
   }
#define RWDT_PROP(name, flags)  DEFINEPROP(name, flags | PF_fSetMsg, 0, 0)
#define RWRT_PROP(name, flags)  RWDT_PROP(name, flags | PF_fNoShow)

extern HANDLE GetSelText(HWND hWnd);
extern BOOL   RegisterVBDate(HANDLE hInstance);
extern BOOL   RegisterVBFloat(HANDLE hInstance);
extern BOOL   RegisterVBInt(HANDLE hInstance);
extern BOOL   RegisterVBPic(HANDLE hInstance);
extern BOOL   RegisterVBSpread(HANDLE hInstance);
extern BOOL   RegisterVBTime(HANDLE hInstance);
extern BOOL   RegisterVBView(HANDLE hInstance);
BOOL          CheckRuntimeLicense(LPSTR lpszCaption,
                                  LPSTR lpszLicenseFileName);
extern void   DisplayRuntimeLicenseMsg(LPSTR lpszCaption);

#endif
