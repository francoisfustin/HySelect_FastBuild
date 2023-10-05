/*    PortTool v2.2     FPTOOL27.C          */

#include "fptools.h"
#include "string.h"
#include <tchar.h>

WORD ParamTypesAdvance[] = {ET_I2, ET_I2};

EVENTINFO EventAdvance =
   {
   _T("Advance"),
   2,
   4,
   ParamTypesAdvance,
   _T("Direction As Integer, AutoAdvance As Integer")
   };

EVENTINFO EventChange =
   {
   _T("Change"),
   0,
   0,
   NULL,
   NULL
   };

WORD ParamMouseButton[] = {ET_I2};

EVENTINFO EventClick =
   {
   _T("Click"),
   1,
   2,
   ParamMouseButton,
   _T("Button As Integer")
   };

EVENTINFO EventDoubleClick =
   {
   _T("DblClick"),
   1,
   2,
   ParamMouseButton,
   _T("Button As Integer")
   };

WORD ParamButton[] = {ET_I2, ET_I2};

EVENTINFO EventButtonHit =
   {
   _T("ButtonHit"),
   2,
   4,
   ParamButton,
   _T("Button As Integer, NewIndex As Integer")
   };

WORD ParamInvalidAction[] = {ET_I2, ET_I2};
                                                                     // `e`n
EVENTINFO EventInvalidAction =                                       // `e`n
{
   _T("UserError"),                                                  // `e`n
   2,
   4,
   ParamInvalidAction,
   _T("ErrorCode As Integer, AutoBeep As Integer")                   // `e`n
   };

WORD ParamInvalidData[] = {ET_I2};
                                                                     // `e`n
EVENTINFO EventInvalidData =                                         // `e`n
   {
   _T("InvalidData"),                                                // `e`n
   1,
   2,
   ParamInvalidData,
   _T("NextWnd As Integer")                                          // `e`n
   };

WORD ParamChangeMode[] = {ET_I2};
                                                                     // `e`n
EVENTINFO EventChangeMode=                                           // `e`n
   {
   _T("ChangeMode"),                                                 // `e`n
   1,
   2,
   ParamChangeMode,
   _T("EditMode As Integer")                                         // `e`n
   };

WORD ParamPopUp[] = {ET_I2};

EVENTINFO EventPopUp =
   {
   _T("PopUp"),
   1,
   2,
   ParamPopUp,
   _T("Cancel As Integer")
   };

//#ifdef NEWDATE                                                                                                       //A521...
EVENTINFO EventPopUpClose =
   {
   _T("PopUpClose"),
   0,
   0,
   NULL,
   NULL
   };
//#endif

WORD ParamKeyDown[] = {ET_I2, ET_I2};

EVENTINFO EventKeyDown =
   {
   _T("KeyDown"),
   2,
   4,
   ParamKeyDown,
   _T("KeyCode As Integer, Shift As Integer")
   };

WORD ParamKeyPress[] = {ET_I2};

EVENTINFO EventKeyPress =
   {
   _T("KeyPress"),
   1,
   2,
   ParamKeyPress,
   _T("KeyAscii As Integer")
   };                                                                                                //A521
