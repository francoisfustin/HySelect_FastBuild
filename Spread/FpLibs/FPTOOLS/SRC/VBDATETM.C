/*    PortTool v2.2     VBDATETM.C          */

#include "fptools.h"
#include "vbdatetm.h"   
#include "string.h"              
#include <tchar.h>

#if (defined(FP_VB) || defined(FP_OCX))  //not FP_DLL

PROPINFO fpPropShortWeekDayName =                                          // 'p'n
   {
   "ShortDayName",                                                       // 'p'n
   DT_HSZ | PF_fPropArray | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };

PROPINFO fpPropShortMonthName =                                          // 'p'n
   {
   "ShortMonthName",                                                       // 'p'n
   DT_HSZ | PF_fPropArray | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
      
PROPINFO fpPropLongWeekDayName =                                          // 'p'n
   {
   "LongDayName",                                                       // 'p'n
   DT_HSZ | PF_fPropArray | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };

PROPINFO fpPropLongMonthName =                                          // 'p'n
   {
   "LongMonthName",                                                       // 'p'n
   DT_HSZ | PF_fPropArray | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };

#endif //not FP_DLL
      
long FPLIB FP_LongWeekDayName (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   LPTSTR lpsz;
   LPTSTR lpData;
   int dIndex;
#ifdef FP_DLL
   LPFP_INDEXPROP_INFO lpIP = (LPFP_INDEXPROP_INFO)lpPP->lParam;
   
   if (!lpIP)
     return(ERR_INVALIDBUFFER);
   dIndex  = lpIP->index;
   lpData = lpIP->u.tValue.lpsz;
#else    
   LPDATASTRUCT lpDs = (LPDATASTRUCT)lpPP->lParam;
   
   dIndex = (short)lpDs->index[0].data;
   lpData = (LPTSTR)lpDs->data;
#endif  //if-else FP_DLL


   if (dIndex < 1 || dIndex > 7)
#ifdef FP_DLL
      return (ERR_INVALIDINDEX);
#else   
      return (381);
#endif      

   lpsz = DateDayName((short)dIndex, TRUE);

   if (fGetProp)
#ifdef FP_DLL
      {
      if (lpData)
        _ftcsncpy( lpData, lpsz, lpIP->u.tValue.maxlen);
      }  
#else   
      lpDs->data = (LONG)VBCreateHsz((_segment)lpPP->hCtl, lpsz);
#endif      
   else 
      {
      _ftcsncpy(lpsz, lpData, 64);
      lpsz[64] = (char)0;
      }

   return (0);
}

long FPLIB FP_LongMonthName (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   LPTSTR lpsz;
   LPTSTR lpData;
   int dIndex;
#ifdef FP_DLL
   LPFP_INDEXPROP_INFO lpIP = (LPFP_INDEXPROP_INFO)lpPP->lParam;
   
   if (!lpIP)
     return(ERR_INVALIDBUFFER);
   dIndex  = lpIP->index;
   lpData = lpIP->u.tValue.lpsz;
#else    
   LPDATASTRUCT lpDs = (LPDATASTRUCT)lpPP->lParam;
   
   dIndex = (short)lpDs->index[0].data;
   lpData = (LPTSTR)lpDs->data;
#endif  //if-else FP_DLL

   if (dIndex < 1 || dIndex > 12)
#ifdef FP_DLL
      return (ERR_INVALIDINDEX);
#else   
      return (381);
#endif      

   lpsz = DateMonthName((short)dIndex, TRUE);

   if (fGetProp)
#ifdef FP_DLL
      {
      if (lpData)
        _ftcsncpy( lpData, lpsz, lpIP->u.tValue.maxlen);
      }  
#else   
      lpDs->data = (LONG)VBCreateHsz((_segment)lpPP->hCtl, lpsz);
#endif      
   else 
      {
      _ftcsncpy(lpsz, lpData, 64);
      lpsz[64] = (char)0;
      }

   return (0);
}

long FPLIB FP_ShortWeekDayName (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   LPTSTR lpsz;
   LPTSTR lpData;
   int dIndex;
#ifdef FP_DLL
   LPFP_INDEXPROP_INFO lpIP = (LPFP_INDEXPROP_INFO)lpPP->lParam;
   
   if (!lpIP)
     return(ERR_INVALIDBUFFER);
   dIndex  = lpIP->index;
   lpData = lpIP->u.tValue.lpsz;
#else    
   LPDATASTRUCT lpDs = (LPDATASTRUCT)lpPP->lParam;
   
   dIndex = (short)lpDs->index[0].data;
   lpData = (LPTSTR)lpDs->data;
#endif  //if-else FP_DLL

   if (dIndex < 1 || dIndex > 7)
#ifdef FP_DLL
      return (ERR_INVALIDINDEX);
#else   
      return (381);
#endif      

   lpsz = DateDayName((short)dIndex, FALSE);

   if (fGetProp)
#ifdef FP_DLL
      {
      if (lpData)
        _ftcsncpy( lpData, lpsz, lpIP->u.tValue.maxlen);
      }  
#else   
      lpDs->data = (LONG)VBCreateHsz((_segment)lpPP->hCtl, lpsz);
#endif      
   else 
      {
      _ftcsncpy(lpsz, lpData, 64);
      lpsz[64] = (char)0;
      }

   return (0);
}

long FPLIB FP_ShortMonthName (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   LPTSTR lpsz;
   LPTSTR lpData;
   int dIndex;
#ifdef FP_DLL
   LPFP_INDEXPROP_INFO lpIP = (LPFP_INDEXPROP_INFO)lpPP->lParam;
   
   if (!lpIP)
     return(ERR_INVALIDBUFFER);
   dIndex  = lpIP->index;
   lpData = lpIP->u.tValue.lpsz;
#else    
   LPDATASTRUCT lpDs = (LPDATASTRUCT)lpPP->lParam;
   
   dIndex = (short)lpDs->index[0].data;
   lpData = (LPTSTR)lpDs->data;
#endif  //if-else FP_DLL

   if (dIndex < 1 || dIndex > 12)
#ifdef FP_DLL
      return (ERR_INVALIDINDEX);
#else   
      return (381);
#endif      

   lpsz = DateMonthName((short)dIndex, FALSE);

   if (fGetProp)
#ifdef FP_DLL
      {
      if (lpData)
        _ftcsncpy( lpData, lpsz, lpIP->u.tValue.maxlen);
      }  
#else   
      lpDs->data = (LONG)VBCreateHsz((_segment)lpPP->hCtl, lpsz);
#endif      
   else 
      {
      _ftcsncpy(lpsz, lpData, 64);
      lpsz[64] = (char)0;
      }

   return (0);
}
