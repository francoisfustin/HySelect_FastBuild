/*************
* VBSS_DEF.H
*************/

#ifndef VBSS_DEF_H
#define VBSS_DEF_H

#define SSP_NO_CURSORICON

#define SSE_NO_INSDEL
#define SSE_NO_ARROWPRESSED

#ifdef SS_V30
#define SS_SDNAME       "sd16d30"      // Name of interface designer
#define SS_DEFCTRLNAME  "vaSpread"     // Default control name
#define SS_VBCLASSNAME  "vaSpread"     // VB class name
#define SS_VBXNAME      "SPREAD30.VBX" // VBX file name
#define SS_LICNAME      "SPREAD25.LIC" // License file name
#define SS_HELPFILENAME "spread30.hlp"   // Name of help file
#else
#define SS_SDNAME       "sd16d25"      // Name of interface designer
#define SS_DEFCTRLNAME  "vaSpread"     // Default control name
#define SS_VBCLASSNAME  "vaSpread"     // VB class name
#define SS_VBXNAME      "SPREAD25.VBX" // VBX file name
#define SS_LICNAME      "SPREAD25.LIC" // License file name
#define SS_HELPFILENAME "ssovx25.hlp"   // Name of help file
#endif

#endif
