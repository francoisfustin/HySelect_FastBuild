/****************************************************************************
*                                                                           *
* ZMOUSE.H -- Include file for IntelliMouse(tm) 1.0                         *
*                                                                           *
* NOTE:  Zmouse.h contains #defines required when providing IntelliMouse    *
*        wheel support for Windows95 and NT3.51.  Wheel is supported        *
*        natively in WinNT4.0, please refer to the NT4.0 SDK for more info  *
*        on providing support for IntelliMouse in NT4.0.                    *
*                                                                           *
* Copyright (c) 1983-1996, Microsoft Corp. All rights reserved.             *
*                                                                           *
\***************************************************************************/


/**************************************************************************
	 Client Appplication (API) Defines for Wheel rolling
***************************************************************************/


// Apps need to call RegisterWindowMessage using the #define below to
// get the message number that is sent to the foreground window
// when a wheel roll occurs

#ifdef UNICODE
#define MSH_MOUSEWHEEL L"MSWHEEL_ROLLMSG"
#else
#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
#endif
   // wParam = wheel rotation expressed in multiples of WHEEL_DELTA
   // lParam is the mouse coordinates

#define WHEEL_DELTA      120      // Default value for rolling one notch


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported
                                        // by the OS 
#endif


/**************************************************************************
    Client Appplication (API) Defines for
	   *  determining if wheel support active
	   *  determining # of Scroll Lines
***************************************************************************/

// Class name for MSWHEEL.EXE's invisible window 
// use FindWindow to get hwnd to MSWHEEL
#ifdef UNICODE
#define MOUSEZ_CLASSNAME  L"MouseZ"           // wheel window class  
#define MOUSEZ_TITLE      L"Magellan MSWHEEL" // wheel window title
#else
#define MOUSEZ_CLASSNAME  "MouseZ"            // wheel window class  
#define MOUSEZ_TITLE      "Magellan MSWHEEL"  // wheel window title
#endif

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

// Apps need to call RegisterWindowMessage using the #defines 
// below to get the message numbers for:
// 1) the message that can be sent to the MSWHEEL window to
//    query if wheel support is active (MSH_WHEELSUPPORT)>
// 2) the message to query for the number of scroll lines 
//    (MSH_SCROLL_LINES)  
//
// To send a message to MSWheel window, use FindWindow with the #defines
// for CLASS and TITLE above.  If FindWindow fails to find the MSWHEEL
// window or the return from SendMessage is false, then Wheel support
// is not currently available.

#ifdef UNICODE
#define MSH_WHEELSUPPORT L"MSH_WHEELSUPPORT_MSG" // name of msg to send
                                                 // to query for wheel support
#else
#define MSH_WHEELSUPPORT "MSH_WHEELSUPPORT_MSG"  // name of msg to send
                                                 // to query for wheel support
#endif

// MSH_WHEELSUPPORT
//    wParam - not used 
//    lParam - not used
//    returns BOOL - TRUE if wheel support is active, FALSE otherwise

                                
#ifdef UNICODE
#define MSH_SCROLL_LINES L"MSH_SCROLL_LINES_MSG"
#else
#define MSH_SCROLL_LINES "MSH_SCROLL_LINES_MSG"
#endif

// MSH_SCROLL_LINES
//    wParam - not used 
//    lParam - not used
//    returns int  - number of lines to scroll on a wheel roll

#ifndef  WHEEL_PAGESCROLL  
#define WHEEL_PAGESCROLL  (UINT_MAX)   // signifies to scroll a page, also
                                       // defined in winuser.h in the
                                       // NT4.0 SDK
#endif 

#ifndef SPI_SETWHEELSCROLLLINES
#define SPI_SETWHEELSCROLLLINES   105  // Also defined in winuser.h in the
                                       // NT4.0 SDK, please see the NT4.0 SDK
                                       // documentation for NT4.0 implementation
                                       // specifics.
                                       // For Win95 and WinNT3.51, 
                                       // Mswheel broadcasts the message
                                       // WM_SETTINGCHANGE (equivalent to
                                       // WM_WININICHANGE) when the scroll
                                       // lines has changed.  Applications
                                       // will recieve the WM_SETTINGCHANGE
                                       // message with the wParam set to
                                       // SPI_SETWHEELSCROLLLINES.  When
                                       // this message is recieved the application
                                       // should query Mswheel for the new 
                                       // setting.
#endif
