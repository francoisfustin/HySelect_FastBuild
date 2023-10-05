/************
* FPPINFO.H
************/

#ifndef FPPINFO_H
#define FPPINFO_H

#define fpDT_HLSTR            0x0D
#define fpDT_FONT			  0x40
// Same as DT_HSZ except it indicates a list of choices
#define fpDT_LIST			  0x41
// -> fix for TIB7034 -scl
#define fpDT_DOUBLE			  0x42
// <- fix for TIB7034 -scl

#define fpPF_datatype         0x000000FFL
#define fpPF_fPropArray       0x00000100L
#define fpPF_fSetData         0x00000200L
#define fpPF_fSetMsg          0x00000400L
#define fpPF_fNoShow          0x00000800L
#define fpPF_fNoRuntimeW      0x00001000L
#define fpPF_fGetData         0x00002000L
#define fpPF_fGetMsg          0x00004000L
#define fpPF_fSetCheck        0x00008000L
#define fpPF_fSaveData        0x00010000L
#define fpPF_fSaveMsg         0x00020000L
#define fpPF_fLoadDataOnly    0x20010000L
#define fpPF_fLoadMsgOnly     0x20020000L
#define fpPF_fGetHszMsg       0x00040000L
#define fpPF_fUpdateOnEdit    0x00080000L
#define fpPF_fEditable        0x00100000L
#define fpPF_fPreHwnd         0x00200000L
#define fpPF_fDefVal          0x00400000L
#define fpPF_fNoInitDef       0x00800000L
#define fpPF_fNoRuntimeR      0x02000000L
#define fpPF_fNoMultiSelect   0x04000000L
// new flag for props that use PF_fSaveData
// but are new to this version of the control
// ie. props that are in the new version's FRM files
// but not in FRM files saved with the old version. -scl
#define fpPF_fNewSaveData     0x80000000L 
										  
											

/*************
* Structures
*************/

#if 0
typedef struct tagFPPROPINFO
  {
  PSTR	npszName;
  DWORD fl;                     // PF_ flags
  BYTE	offsetData;		// Offset into static structure
  BYTE	infoData;		// 0 or _INFO value for bitfield
  LONG	dataDefault;		// 0 or _INFO value for bitfield
  PSTR	npszEnumList;		// For type == DT_ENUM, this is
				// a near ptr to a string containing
				// all the values to be displayed
				// in the popup enumeration listbox.
				// Each value is an sz, with an
				// empty sz indicated the end of list.
  BYTE	enumMax;		// Maximum legal value for enum.
  } FP_PROPINFO, NEAR *PFP_PROPINFO;
#endif

#define FP_PROPINFO  PROPINFO
#define PFP_PROPINFO PPROPINFO

#define fpIPROP_STD_LAST      (UINT)0x0FFF
#define fpPPROPINFO_STD_LAST  ((PFP_PROPINFO)~fpIPROP_STD_LAST)

#ifdef FP_OCX
#define fpPPROPINFO_STD_BACKCOLOR ((PFP_PROPINFO)&NCPropBackColor)
#define fpPPROPINFO_STD_FORECOLOR ((PFP_PROPINFO)&NCPropForeColor)
#define fpVB_STD  0
#else  //FP_VB
#define fpPPROPINFO_STD_BACKCOLOR PPROPINFO_STD_BACKCOLOR
#define fpPPROPINFO_STD_FORECOLOR PPROPINFO_STD_FORECOLOR
#define fpVB_STD  -1
#endif

#define FP_ALL       (WORD)0x00
#define FP_NO_DLL    (WORD)0x01
#define FP_NO_OCX    (WORD)0x02
#define FP_VALLIST   (WORD)0x04  // Indicate to fpPropDesigner that prop has a
                               // list of possible values. No relation to old "FP_STR_GH".
#define FP_BLOB      (WORD)0x08
#define FP_NOTINBLOB (WORD)0x80 // indicates to prop designer subclasses that this prop
								// is NOT part of the blob data.  not used by the prop designer
								// base class or the get/set funcs in fptools. -scl
#ifdef FP_OCX      // Here we will re-use bits for exclusive environments
#define FP_NULLDPTBL (WORD)0x10 // Empty (null) entry in OCX Dispatch table - SCP.
#define FP_NODPTBL   (WORD)0x20 // NO entry in OCX Dispatch table - SCP.
#define FP_NO_VB1    (WORD)0    // not used in OCX...
#define FP_NO_VB2    (WORD)0
#define FP_NO_VB3    (WORD)0
#define FP_NO_VB     (WORD)0
#else // FP_VB
#define FP_NO_VB1    (WORD)0x10
#define FP_NO_VB2    (WORD)0x20
#define FP_NO_VB3    (WORD)0x40
#define FP_NO_VB     (WORD)0x70
#define FP_NULLDPTBL (WORD)0    // not used in VBX...
#define FP_NODPTBL   (WORD)0    // 
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif  /* RC_INVOKED */

typedef struct tagFP_PROPLIST
   {
   char         bDataType;
   PFP_PROPINFO pPropInfo;
   WORD         wEnvironment;
   short        sDispid;
   } FP_PROPLIST, FAR *LPFP_PROPLIST;

#ifdef FP_OCX   //if OCX

#define SPROPINFO_STD_FONTNAME(name)  \
  static PROPINFO name =              \
   {                                                               \
   _T("FontName"),                                                 \
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fSaveData |               \
   PF_fNoInitDef, 0, 0, 0, 0, 0                                    \
   }

#define SPROPINFO_STD_FONTBOLD(name)  \
  static PROPINFO name =              \
   {                                                               \
   _T("FontBold"),                                                 \
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | \
   PF_fNoInitDef, 0, 0, FALSE, 0, 0                                \
   }

#define SPROPINFO_STD_FONTITALIC(name)  \
  static PROPINFO name =                \
   {                                                               \
   _T("FontItalic"),                                               \
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | \
   PF_fNoInitDef, 0, 0, FALSE, 0, 0                                \
   }

#define SPROPINFO_STD_FONTSTRIKE(name)  \
  static PROPINFO name =                \
   {                                                               \
   _T("FontStrikethru"),                                           \
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | \
   PF_fNoInitDef, 0, 0, FALSE, 0, 0                                \
   }

#define SPROPINFO_STD_FONTUNDER(name)   \
  static PROPINFO name =                \
   {                                                                \
   _T("FontUnderline"),                                             \
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |  \
   PF_fNoInitDef, 0, 0, FALSE, 0, 0                                 \
   }

#define SPROPINFO_STD_FONTSIZE(name)    \
  static PROPINFO name =                \
   {                                                               \
   _T("FontSize"),                                                 \
   DT_REAL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData |              \
   PF_fNoInitDef, 0, 0, -1, 0, 0                                   \
   }

#endif  //FP_OCX

#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */

#endif
