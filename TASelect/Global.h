//
// Global.h : Useful macros 
//
/////////////////////////////////////////////////////////////////////////////

#ifndef GLOBAL_H__INCLUDED_
#define GLOBAL_H__INCLUDED_
#include <iostream>
#include <fstream>
#include <sstream> 
#include <string> 
#include <map>
#include <set>
#include <vector>
#define _USE_MATH_DEFINES
#include "Math.h"

using namespace std;

#define OUTSTREAM std::ofstream&
#define INPSTREAM std::ifstream&

//////////////////////////////////////////TA CBX/////////////////////////////
#ifdef TACBX

#define ASSERT(f)			


#define TRACE(sz)			
#define TRACE0(sz)
#define TRACE1(sz, p1)
#define TRACE2(sz, p1, p2)
#define TRACE3(sz, p1, p2, p3)
//
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif
//////////////////////////////////////////TA CBX/////////////////////////////


#ifdef UNICODE
#define _string std::wstring 
#define _tistream basic_istream<wchar_t, char_traits<wchar_t>>
#define _tostream basic_ostream<wchar_t, char_traits<wchar_t>>
#define _tostringstream basic_ostringstream<wchar_t, char_traits<wchar_t>, allocator<wchar_t>>
#else
#define _string std::string 
#define _tistream basic_istream<char, char_traits<char>>
#define _tostream basic_ostream<char, char_traits<char>>
#define _tostringstream basic_ostringstream<char, char_traits<char>, allocator<char>>;
#endif

//Disable warning C4482: nonstandard extension used: enum 'C...::...' used in qualified name
#pragma warning( disable : 4482)
#pragma warning( disable : 4091)

/////////////////////////////////////////////////////////////////////////////////////////
//	Features
////////////////////////////////////////////////////////////////////////////////////////
// Comment this line if you don't want the Import/Export json feature.
#define _JSON_FEATURE_ENABLED

/////////////////////////////////////////////////////////////////////////////////////////
//	TA_COLOR Definition
////////////////////////////////////////////////////////////////////////////////////////
#define _TAH_COLORTOREPLACE_1	_VIOLET
#define _TAH_COLORTOREPLACE_2	_LILA

#define _TAH_WHITE				RGB(255,255,255)

#define _TAH_ORANGE				RGB(220,115,0)
#define _TAH_ORANGE_MED			RGB(242,150,51)
#define _TAH_ORANGE_LIGHT		RGB(246,177,102)
#define _TAH_ORANGE_XLIGHT		RGB(249,203,153)
#define _TAH_ORANGE_XXLIGHT		RGB(255,242,183)

#define _IMI_GRAY				RGB(87,87,87)
#define _IMI_GRAY_MED			RGB(135,135,135)
#define _IMI_GRAY_LIGHT			RGB(180,180,180)
#define _IMI_GRAY_XLIGHT		RGB(220,220,220)
#define _IMI_GRAY_XXLIGHT		RGB(240,240,240)

#define _TAH_BERYLLIUM			RGB(112,144,183)
#define _TAH_BERYLLIUM_MED		RGB(130,167,197)
#define _TAH_BERYLLIUM_LIGHT	RGB(161,189,212)
#define _TAH_BERYLLIUM_XLIGHT	RGB(192,211,226)

#define _TAH_BLACK				RGB(0,0,0)
#define _TAH_BLACK_MED			RGB(86,86,86)
#define _TAH_BLACK_LIGHT		RGB(135,135,135)
#define _TAH_BLACK_XLIGHT		RGB(177,178,179)

#define _TAH_RED				RGB(180,65,65)
#define _TAH_RED_MED			RGB(199,97,96)
#define _TAH_RED_LIGHT			RGB(213,137,136)
#define _TAH_RED_XLIGHT			RGB(227,176,176)

#define _TAH_BLUE				RGB(45,80,120)
#define _TAH_BLUE_MED			RGB(92,119,149)
#define _TAH_BLUE_LIGHT			RGB(133,153,176)
#define _TAH_BLUE_XLIGHT		RGB(174,187,202)

#define _TAH_GREY				RGB(188,175,165)
#define _TAH_GREY_MED			RGB(210,201,192)
#define _TAH_GREY_LIGHT			RGB(221,214,208)
#define _TAH_GREY_XLIGHT		RGB(232,228,224)

#define _TAH_BROWN				RGB(103,64,53)
#define _TAH_BROWN_MED			RGB(135,99,89)
#define _TAH_BROWN_LIGHT		RGB(165,138,130)
#define _TAH_BROWN_XLIGHT		RGB(195,177,172)

#define _TAH_GREEN				RGB(95,140,95)
#define _TAH_GREEN_MED			RGB(135,173,135)
#define _TAH_GREEN_LIGHT		RGB(166,196,166)
#define _TAH_GREEN_XLIGHT		RGB(199,218,199)

#define _TAH_TITLE_MAIN				_TAH_ORANGE
#define _TAH_TITLE_SECOND			_IMI_GRAY_MED
#define _TAH_UNDER_SELECTION		_TAH_GREY_XLIGHT//_TAH_BERYLLIUM_LIGHT//_TAH_GREY_XLIGHT//_TAH_BLUE_XLIGHT//_TAH_ORANGE_XLIGHT
#define _TAH_SELECTED_ORANGE		_TAH_ORANGE_XLIGHT
#define _TAH_SELECTED_BLUE			_TAH_GREY_XLIGHT//_TAH_BERYLLIUM_LIGHT

#define _TAH_TITLE_MAIN_REEDIT		_TAH_RED
#define _TAH_TITLE_SECOND_REEDIT	_TAH_BLUE
#define _TAH_TITLE_MAIN_REEDIT_L	_TAH_RED_LIGHT
#define _TAH_TITLE_SECOND_REEDIT_XL	_TAH_BLUE_LIGHT

#define _IMI_TITLE_GROUP1			_IMI_GRAY_MED
#define _IMI_TITLE_GROUP2			_IMI_GRAY_LIGHT
#define _IMI_TITLE_GROUP3			_IMI_GRAY_LIGHT
#define _IMI_TITLE_GROUPHEATING		RGB( 232, 78, 64 )
#define _IMI_TITLE_GROUPCOOLING		RGB( 68, 138, 255 )

#define _TAH_DIVERSITYFACTOR	 _TAH_GREEN_XLIGHT	

#define _RED					RGB(255,0,0)
#define _DARKRED				RGB(140,67,58)
#define _LIGHTRED				RGB(196,158,154)

#define _ORANGE					RGB(255,102,0)

#define _VIOLET					RGB(255,0,255)
#define _LILA					RGB(192,192,255)
#define _LIGHTLILA				RGB(198,200,224)

#define _YELLOW					RGB(255,255,0)
#define _MEDIUMYELLOW			RGB(255,255,127)
#define _LIGHTYELLOW			RGB(255,255,196)
#define _VLIGHTYELLOW			RGB(255,255,220)

#define _BLUE					RGB(0,0,128)
#define _LIGHTBLUE				RGB(0,0,255)
#define _SELECTEDBLUE			RGB(49,106,197)
#define _VLBLUE					RGB(156,172,220)

#define _GREEN					RGB(0,128,0)
#define _DARKGREEN				RGB(0,96,0)
#define _SOFTLIGHTGREEN			RGB(0,200,0)
#define _LIGHTGREEN				RGB(0,255,0)

#define _WHITE					RGB(255,255,255)
#define _WHITE_DLGBOX			RGB(252,252,254)
#define _BLACK					RGB(0,0,0)

#define _GRAY					RGB(192,192,192)
#define _VLIGHTGRAY				RGB(240,240,240)
#define _LIGHTGRAY				RGB(220,220,220)
#define _DARKGRAY				RGB(128,128,128)
#define _VDARKGRAY				RGB(96,96,96)

#define _INDEXCOLOR				_LIGHTBLUE
#define _LIGHTINDEXCOLOR		RGB( 173, 173, 255 )

#define _EDITCOLOR				RGB( 234, 215, 255 )

/////////////////////////////////////////////////////////////////////////////////////////
//	Timer ID Definition
//
//  Avoid to use 42 and 43 that are reserved for some MFC controls.
////////////////////////////////////////////////////////////////////////////////////////
#define _TIMERID_MAIN								0x1000
#define _TIMERID_DLGSELECTACTUATOR					0x1001
#define _TIMERID_DLGSPREADCB						0x1002
#define _TIMERID_DLGPANELTASCOPEDOWNLOAD			0x1003
#define _TIMERID_DLGPANELTASCOPEMAINTENANCE			0x1004
#define _TIMERID_DLGPANELTASCOPESTART				0x1005
#define _TIMERID_DLGPANELTASCOPEUPDATE				0x1006
#define _TIMERID_DLGPANELTASCOPEUPLOAD				0x1007
#define _TIMERID_DLGWIZTASCOPE						0x1008
#define _TIMERID_HMTREELISTCTRL						0x1009
#define _TIMERID_PANELCIRC1							0x100A
#define _TIMERID_PANELCIRC2							0x100B
#define _TIMERID_PROJTREECTRL						0x100C
#define _TIMERID_SPLASHWINDOW						0x100D
#define _TIMERID_SHEETSELAUTOSCROLL					0x100E
#define _TIMERID_DLGDPSVISIOUNPLUGPLUG				0x100F
#define _TIMERID_DLGDPSVISIODFUPOLLINGDETECTION		0x1010
#define _TIMERID_DLGDPSVISIOUPGRADERUNNINGPUTMSG	0x1011

///////////////////////////////////////////////////////
//	SYNCHOBJ is used to prevent both instance of HySelect
//			 is used by HyUpdate to determine before copying files 
//			 if HySelect is still running
///////////////////////////////////////////////////////

#define SYNCHOBJ					_T("TA_SelectSynchObj")


///////////////////////////////////////////////////////
//			Define window messages
///////////////////////////////////////////////////////

// Notes from Microsoft documentation

// Message numbers in the second range (WM_USER through 0x7FFF) can be defined and used by an application to send messages 
// within a private window class. These values cannot be used to define messages that are meaningful throughout an application, 
// because some predefined window classes already define values in this range. For example, predefined control classes such as 
// BUTTON, EDIT, LISTBOX, and COMBOBOX may use these values. Messages in this range should not be sent to other applications 
// unless the applications have been designed to exchange messages and to attach the same meaning to the message numbers. 

// Message numbers in the third range (0x8000 through 0xBFFF) are available for application to use as private messages.
// Message in this range do not conflict with system messages. 

#define WM_OFFSET								( WM_USER + 0x0700 )	// WM_USER = 0x0400
#define WM_USER_UNITCHANGE						( WM_OFFSET)			// notify when default units are changed.
#define WM_USER_WATERCHANGE						( WM_OFFSET + 1 )		// notify when water characteristics are changed.
#define WM_USER_NEWDOCUMENT						( WM_OFFSET + 2 )		// notify when a new document is created or loaded.
#define WM_USER_TECHPARCHANGE					( WM_OFFSET + 3 )		// notify when technical parameters are changed.
#define WM_USER_PIPECHANGE						( WM_OFFSET + 4 )		// notify when pipe database is modified.
#define WM_USER_CHANGELVTAB						( WM_OFFSET + 5 )		// notify to change the left view tab selection.
#define WM_USER_CHANGEFOCUSRVIEWSSEL			( WM_OFFSET + 6 )		// notify when the focus in RightViewSSel changes
#define WM_USER_SSELITEMSELECTED				( WM_OFFSET + 7 ) 		// notify when a SSel item is kept as a selection.
#define WM_USER_DESTROYDIALOGINFOSSELBV			( WM_OFFSET + 8 ) 		// notify that the DialogInfoSSelBv modeless dialog must be destroyed.
#define WM_USER_DESTROYDIALOGINFOSSELDPC		( WM_OFFSET + 9 ) 		// notify that the DialogInfoSSelDpC modeless dialog must be destroyed.
#define WM_USER_CLOSEAPPLICATION				( WM_OFFSET + 10 )		// notify that the user close the application
#define WM_USER_CHECKSTATECHANGE				( WM_OFFSET + 11 )		// notify that the user click on a treeview , used for check or uncheck checkbox associated 
#define WM_USER_DISPLAYDOCUMENTATION			( WM_OFFSET + 12 )		// User ask to see a product related documentation
#define WM_USER_UPDATEPRODUCTDATABASE			( WM_OFFSET + 13 )		// notify that it's necessary to launch FtpUpdate 
#define WM_USER_USERSELECTIONCHANGE				( WM_OFFSET + 14 )		// notify that the RightViewSelp must be drawn
#define WM_USER_DESTROYDIALOGADDVALVE			( WM_OFFSET + 15 )		// notify that the DialogAddValve modeless dialog must be destroyed.
#define WM_USER_INITCOMPORT						( WM_OFFSET + 16 )		// notify that all window initialization is done and you can now initialize the com port
#define WM_USER_DATATRANSFERREADY				( WM_OFFSET + 17 )		// notify all conditions are done for data transfer (PanelCBI3)
#define WM_USER_PROJECTREFCHANGE				( WM_OFFSET + 18 )		// notify when the project reference is changed
#define WM_USER_ENDPRINTPREVIEW					( WM_OFFSET + 19 )		// notify when PrintPreview windows is close used in print dialog
#define WM_USER_DESTROYDIALOGHMTREE				( WM_OFFSET + 20 )		// notify that the modeless DialogHMTree dialog must be destroyed.
#define WM_USER_CLOSESPREADCOMBOBOX				( WM_OFFSET + 21 )		// notify that the specific combobox spread must be closed.
#define WM_USER_ENTERKEYPRESSED					( WM_OFFSET + 22 )		// User press enter key in a CExtNumEdit.
#define WM_USER_PSWCHANGED						( WM_OFFSET + 23 )		// notify that the user enter the password to get access to hidden products.
#define WM_USER_HMTREEITEMINSERTED				( WM_OFFSET + 24 )		// notify when an hItem has been inserted into an HMTree.
#define WM_USER_HMTREEITEMCHECKCHANGED			( WM_OFFSET + 25 )		// notify when an hItem check status has changed into an HMTree.
#define WM_USER_REPLACEPOPUPFINDNEXT			( WM_OFFSET + 26 )		// notify when Find next btn has been pressed by DlgReplacePopup.
#define WM_USER_REPLACEPOPUPREPLACE				( WM_OFFSET + 27 )		// notify when Replace btn has been pressed by DlgReplacePopup.
#define WM_USER_REPLACEPOPUPREPLACEALL			( WM_OFFSET + 28 )		// notify when Replace All btn has been pressed by DlgReplacePopup.
#define WM_USER_HMTREEMODIFIED					( WM_OFFSET + 29 )		// notify when an hItem has been added or deleted
#define WM_USER_MODIFYSELECTEDBCV				( WM_OFFSET + 30 )		// notify that user requested a modification of a balancing and control valve.
#define WM_USER_MODIFYSELECTEDBV				( WM_OFFSET + 31 )		// notify that user requested a modification of a Balancing Valve.
#define WM_USER_MODIFYSELECTEDCV				( WM_OFFSET + 32 )		// notify that user requested a modification of a Control Valve.
#define WM_USER_MODIFYSELECTEDDPC				( WM_OFFSET + 33 )		// notify that user requested a modification of a Dp Controller.
#define WM_USER_MODIFYSELECTEDHUB				( WM_OFFSET + 34 )		// notify that user requested a modification of a selected hub.
#define WM_USER_MODIFYSELECTEDPICV				( WM_OFFSET + 35 )		// notify that user requested a modification of a Pressure independent control valve.
#define WM_USER_MODIFYSELECTEDTRV				( WM_OFFSET + 36 )		// notify that user requested a modification of a TRV.
#define WM_USER_MODIFYSELECTEDSEPARATOR			( WM_OFFSET + 37 )		// notify that user requested a modification of a separator.
#define WM_USER_MODIFYSELECTEDPM				( WM_OFFSET + 38 )		// notify that user requested a modification of a pressure maintenance product.
#define WM_USER_MODIFYSELECTEDDPCBCV			( WM_OFFSET + 39 )		// notify that user requested a modification of a combined Dp controller and balancing & control valve.
#define WM_USER_MODIFYSELECTEDSV				( WM_OFFSET + 40 )		// notify that user requested a modification of a combined Dp controller and balancing & control valve.
#define WM_USER_MODIFYSELECTEDSAFETYVALVE		( WM_OFFSET + 41 )		// notify that user requested a modification of a safety valve.
#define WM_USER_MODIFYSELECTED6WAYVALVE			( WM_OFFSET + 42 )		// notify that user requested a modification of a 6-way valve.
#define WM_USER_MODIFYSELECTEDSMARTCONTROLVALVE		( WM_OFFSET + 43 )		// notify that user requested a modification of a smart control valve.
#define WM_USER_MODIFYSELECTEDSMARTDPC		    ( WM_OFFSET + 44 )		// notify that user requested a modification of a smart differential controller valve.
#define WM_USER_SSHEETCOLWIDTHCHANGE			( WM_OFFSET + 45 )		// notify that CDlgIndSelTrv can relaunch a selection.
#define WM_USER_CLOSEPOPUPWND					( WM_OFFSET + 46 )		// notify when user close the popup wnd
#define WM_USER_RESETHMTREE						( WM_OFFSET + 47 )		// notify that tree in CDlgLeftTabProject must be reseted.
#define WM_USER_DIVERSITYFACTORTOCLOSE			( WM_OFFSET + 48 )		// notify that user close the diversity factor dialog.
#define WM_USER_DLGPRINTTOCLOSE					( WM_OFFSET + 49 )   	// notify that user close the print dialog.
#define WM_USER_DLGDEBUGCLOSE					( WM_OFFSET + 50 )		// notify that user close the debug dialog.
#define WM_USER_DELETETSPREADINSTANCE			( WM_OFFSET + 51 )		// To postpone deletion of TSpread (with a 'PostMessage' instead of 'SendMessage').
#define WM_USER_SHOWPOPUPMENU					( WM_OFFSET + 52 )		// notify a popup menu to display.
#define WM_USER_MESSAGEDATA						( WM_OFFSET + 53 )		// new message to allow to send more complete messages (see 'MessageManager').
#define WM_USER_SELPRODEDITPRODUCT				( WM_OFFSET + 54 )		// notify that the right view can edit the selected product in the current 'SelProdPageXXX'.
#define WM_USER_MAINFRAMERESIZE					( WM_OFFSET + 55 )		// notify that the main frame has been resized.
#define WM_USER_PMWQSELECTIONPREFSCHANGE		( WM_OFFSET + 56 )		// notify that user has changed something in the 'Pressurization & Water Quality preferences' in the ribbon or the dialog.
#define WM_USER_DISPLAYMSGBOX					( WM_OFFSET + 57 )		// Display a message box by using message heap
#define WM_USER_RESETHMTREECREATE				( WM_OFFSET + 58 )		// notify that tree in CDlgLeftTabProject must be reseted during creation.
#define WM_USER_RESETHMTREEEDIT					( WM_OFFSET + 59 )		// notify that tree in CDlgLeftTabProject must be reseted during edition.
#define WM_USER_OPENDOCUMENT					( WM_OFFSET + 60 )		// Open tsp document from unit test
#define WM_USER_EXPORTSELPTOXLSX				( WM_OFFSET + 61 )		// Save Product Selection to an XLSX file
#define WM_USER_DESTROYDIALOGINFOSSELDPCBCV		( WM_OFFSET + 62 ) 		// notify that the DialogInfoSSelDpCBCV modeless dialog must be destroyed.
#define WM_USER_DPSVISIO_FORCEHIDDETECTION		( WM_OFFSET + 63 )		// notify that we want to check the DpS-Visio HID connection status.
#define WM_USER_DPSVISIO_SHOWDIALOG				( WM_OFFSET + 64 )		// notify that 'CDlgDpSVisio' can change the current dialog.
#define WM_USER_DPSVISIO_STOPTHREADUPGRADE		( WM_OFFSET + 65 )		// notify to the 'CDlgDpSVisioHIDUpgradeRunning' that thread is finished.
#define WM_USER_DPSVISIO_ENABLEBUTTON			( WM_OFFSET + 66 )		// notify to the 'CDlgDpSVisio' to enable/disable a button.
#define WM_USER_DPSVISIO_SHOWDLGUNPLUGPLUG		( WM_OFFSET + 67 )		// notify that 'CDlgDpSVisioHIDUpgradeRunning' must create CDlgDpSVisioUnplugPlug.
#define WM_USER_DPSVISIO_STOPDLGUNPLUGPLUG		( WM_OFFSET + 68 )		// notify that 'CDlgDpSVisioHIDUpgradeRunning' must destroy CDlgDpSVisioUnplugPlug.
#define WM_USER_DPSVISIO_FISNISHDLGUNPLUGPLUG 	( WM_OFFSET + 69 )		// notify that 'CDlgDpSVisioHIDUpgradeRunning' must finish CDlgDpSVisioUnplugPlug.
#define WM_USER_RIGHTVIEWWIZPM_SWITCHMODE		( WM_OFFSET + 70 )		// notify that 'CRViewWizardSelPM' must change the input dialog.
#define WM_USER_RIGHTVIEWWIZPM_BACKNEXT			( WM_OFFSET + 71 )		// notify to 'CRViewWizardSelPM' that user clicks on the 'Back' or 'Next' button.
#define WM_USER_RIGHTVIEWWIZPM_INPUTCHANGE		( WM_OFFSET + 72 )		// notify to 'CRViewWizardSelPM' that user has changed a value.
#define WM_USER_APPLICATIONTYPECHANGE			( WM_OFFSET + 73 )		// notify to mainframe that application type (Heating, cooling or solar) for pressurisation has changed.
#define WM_USER_MODIFYSELECTEDWIZPM				( WM_OFFSET + 74 )		// notify that user requested a modification of a pressure maintenance product in wizard mode.
#define WM_USER_DPSVISIO_SHOWDLGDFUINSTALLATION ( WM_OFFSET + 75 )		// notify that 'CDlgDpSVisioHIDUpgradeRunning' must create CDlgDpSVisioDFUInstallation.
#define WM_USER_DPSVISIO_FISNISHDFUINSTALLATION ( WM_OFFSET + 76 )		// notify that 'CDlgDpSVisioHIDUpgradeRunning' must destroy CDlgDpSVisioUnplugPlug.
#define WM_USER_DPSVISIO_EXIT					( WM_OFFSET + 77 )		// notify that 'CDlgDpSVisio' must stop all and exit.
#define WM_USER_EXPORTSELPINONETOXLSX			( WM_OFFSET + 78 )		// Export Product Selection to an XLSX file with one sheet for valves and one for pneumatex.
#define WM_USER_IMPORT_UT						( WM_OFFSET + 79 )		// Allow to call import process from unit test and create the dialog.
#define WM_USER_WIZARDMANAGER_DOBACK			( WM_OFFSET + 80 )		// notify to 'CDlgWizardManager' that it must go back.

///////////////////////////////////////////////////////
//			Definition of enums for user message
///////////////////////////////////////////////////////

// Parameter for the WParam for the 'WM_USER_WATERCHANGE' user message.
enum WMUserWaterCharWParam
{
	WM_UWC_WP_ForProject		= 1,
	WM_UWC_WP_ForProductSel		= 2,
	WM_UWC_WP_ForAll			= ( WM_UWC_WP_ForProductSel | WM_UWC_WP_ForProject ),
	WM_UWC_WP_ForTools			= 4
};

// Parameter for the LParam for the 'WM_USER_WATERCHANGE' user message.
enum WMUserWaterCharLParam
{
	WM_UWC_LWP_NoChange		                = 0,
	WM_UWC_LWP_Change		                = 1,
	WM_UWC_LWP_NoChangeForChangeOver		= 2,
	WM_UWC_LWP_ChangeForChangeOver		    = 3
};

// Parameter for the WParam for the 'WM_USER_PIPECHANGE' user message.
enum WMUserPipeChange
{
	WM_UPC_ForProductSelection	= 0,
	WM_UPC_ForProject			= 1
};

// Parameter for the WParam of the 'WM_USER_RIGHTVIEWWIZPM_BACKNEXT' user message.
enum WMRightViewWizardPM_BackNext
{
	WM_RVWPMBN_Back	= 0,
	WM_RVWPMBN_Next	= 1
};

enum ProductSelectionMode
{
	ProductSelectionMode_First = 0,
	ProductSelectionMode_Individual = ProductSelectionMode_First,
	ProductSelectionMode_Batch,
	ProductSelectionMode_Wizard,
	ProductSelectionMode_Last = ProductSelectionMode_Wizard
};

///////////////////////////////////////////////////////
//			Define data directory and files
///////////////////////////////////////////////////////
// Application name
#define _HYSELECT_NAME				_T("HySelect")
#define _HYSELECT_NAME_BCKSLASH		_T("\\HySelect\\")
#define _LANG_DEP_BCKSLASH		_T("Language dependant\\")
#define _HYSELECTUSERGUIDE      _T("HySelect User Guide.pdf")
// The data directory
#define _DATA_DIRECTORY			_T("DATA\\")
#define _LOGS_DIRECTORY			_T("LOGS\\")
// The database text file
#define _DATABASE_TEXT_FILE		_T("DATA\\Tadb.txt")
#define _PIPEDB_TEXT_FILE		_T("DATA\\Pipedb.txt")
#define _TADB_REGV_TEXT_FILE	_T("DATA\\Tadb_RegV.txt")
#define _TADB_STADSTAR_TXT_FILE	_T("DATA\\Tadb_STADstar.txt")
#define _TADB_DPC_TEXT_FILE		_T("DATA\\Tadb_DpC.txt")
#define _TADB_TRV_TEXT_FILE		_T("DATA\\Tadb_Trv.txt")
#define _TADB_CV_TEXT_FILE		_T("DATA\\Tadb_CV.txt")
#define _TADB_BCV_TEXT_FILE		_T("DATA\\Tadb_BCV.txt")
#define _TADB_ACT_TEXT_FILE     _T("DATA\\Tadb_Act.txt")
#define _TADB_HUB_TEXT_FILE		_T("DATA\\Tadb_Hub.txt")
#define _TADB_DELETED_PROD		_T("DATA\\Tadb_Deleted.txt")
#define _TADB_WQ_TEXT_FILE		_T("DATA\\Tadb_WQ.txt")
#define _TADB_PM_TEXT_FILE		_T("DATA\\Tadb_PM.txt")
#define _TADB_SV_TEXT_FILE		_T("DATA\\Tadb_ShutoffValve.txt")
#define _TADB_SAFETYVALVE_TEXT_FILE		_T("DATA\\Tadb_SafetyValve.txt")
#define _TADB_SMARTVALVE_TEXT_FILE		_T("DATA\\Tadb_SmartValve.txt")
#define _TADB_STAFSTAR_TXT_FILE		_T("DATA\\Tadb_STAFstar.txt")
#define _TADB_ACC_TEXT_FILE		_T("DATA\\Tadb_Accessories.txt")
// The redefine text file, contains all CDB_TAProduct redefinitions, only used when reading the database text file
#define _REDEFINETAB_TEXT_FILE _T("DATA\\RedefineTab.txt")
// The local article number text file, contains all CDB_TAProduct local article number definitions, only used when reading the database text file
#define _LOCARTNUMTAB_TEXT_FILE _T("DATA\\LanTab.txt")
// The filter text file, contains all filter definitions, only used when reading the database text file
#define _FILTERTAB_TEXT_FILE _T("DATA\\FilterTab.txt")
// The string table XML file,
#define _STRINGTAB_XML_FILE "DATA\\TAdbST.xml"
// The string resource table XML file,
#define _TASRC_XML_FILE "DATA\\TasRc.xml"
// The tips resource table XML file,
#define _TASTIPS_XML_FILE "DATA\\TasTips.xml"
// The TAHUB XsL file,
#define _STYLESHEET_XSL_FILE _T("DATA\\TA-Hub stylesheet.xsl")
// The localized data/objects text file, only used when reading the database text file
#define _LOCALDB_TEXT_FILE _T("DATA\\Localdb.txt")
// The product documentation database text file, only used when reading the database text file
#define _DOCDB_TEXT_FILE _T("DATA\\Docdb.txt")
// The price database text file
#define _PRICEDB_TEXT_FILE _T("DATA\\PriceDB.txt")
// The user product database text file, only used when reading the database text file
#define _USERPRODDB_TEXT_FILE _T("DATA\\Userproddb.txt")
// The database binary file
#define _DATABASE_BIN_FILE _T("DATA\\TADB.DB")
// The doc database binary file
#define _DOC_DATABASE_BIN_FILE _T("DOC\\TADoc.DB")
// The user database text file
#define _USER_DATABASE_TEXT_FILE _T("DATA\\USER.TXT")
// The user database binary file
#define _USER_DATABASE_BIN_FILE _T("DATA\\USER.DB")
// The pipe database binary file
#define _PIPES_DATABASE_BIN_FILE _T("DATA\\PIPES.DB")
// The tender text zip file
#define _TENDER_ZIP_FILE _T("DATA\\tender.zip")
// The docdb csv file
#define _DOCDB_CSV_FILE _T("Doc\\%s\\docdb.csv")
// The Addins directory in which TAS2 looks to see if there is a Service Pack to be executed (after a HyUpdate)
#define _ADDINS_DIRECTORY _T("Addins\\")
// The Project directory
#define _PROJECTS_DIRECTORY _T("Projects\\")
// The Doc directory
#define _DOC_DIRECTORY _T("Doc\\")
// The Help directory
#define _HELP_DIRECTORY _T("Help\\")
// AutoSave Directory
#define _AUTOSAVE_DIRECTORY	_T("Autosave\\")
// The Sub language  filename
#define _SUBLANGUAGE_FILENAME _T("TADB.key")
// Dll names
#define _HUB_SCHEMES_DLL _T("HubSchemes.dll")
#define _HYDRO_PICS _T("HydronicPics.dll")
#define _LOCPROD_PICS _T("LPP")

// TASCOPE
#define _TASCOPE_DIRECTORY	_T("TASCOPE")
#define _TASCOPE_MAINTENANCE_DIRECTORY _T("Maintenance")
#define _TASCOPE_DATA_DIRECTORY _T("Data")
#define _TASCOPE_BACKUP_DIRECTORY _T("Backup")
#define _TASCOPE_HFT _T("hft-tascope.txt")
#define _TASCOPE_SFT _T("sft-tascope.txt")

// DpS-Visio
#define _DPSVISIO_DFU_FILENAME		_T("DPS2.dfu")
#define _DPSVISIO_BIN_FILENAME		_T("DPS2.bin")

// Hub Number of stations
#define HUB_MIN_NUMBEROFSTATION				2
#define HUB_MAX_NUMBEROFSTATION				8

// HUB Description length				
#define HUB_MAXLENGTH_DESCRIPTION			35


///////////////////////////////////////////////////////
//			Macro definition
///////////////////////////////////////////////////////

// Avoid that Enter key closes undesirably a dialog box
#define PREVENT_ENTER_KEY if (GetFocus()!=GetDlgItem(IDOK)) return ;

// Avoid that empty edit boxes prevent canceling a dialog box
#define PREVENT_NOT_CANCEL_WITH_EMPTY_BOX if (GetFocus()==GetDlgItem(IDCANCEL)) return ;

// Return size in characters of a TCHAR buffer
#define SIZEOFINTCHAR(buf) sizeof(buf)/sizeof(TCHAR)

// Try, throw and catch.
#ifndef TACBX
#define HYSELECT_TRY											try
#define HYSELECT_THROW( message, ... )							throw CHySelectException( message, __LINE__, __FILE__, __VA_ARGS__ )
// This one is for the TA-Scope to return variable.
#define HYSELECT_THROW_RETURNARG( message, arg, ... )			throw CHySelectException( message, __LINE__, __FILE__, __VA_ARGS__ )
#define HYSELECT_CATCH( class_name, message )					catch( CHySelectException &class_name ) { class_name.AddMessage( message, __LINE__, __FILE__ ); throw; }
#define HYSELECT_CATCH_ARG( class_name, message, ... )			catch( CHySelectException &class_name ) { class_name.AddMessage( message, __LINE__, __FILE__, __VA_ARGS__ ); throw; }
#else
#define HYSELECT_TRY
#define HYSELECT_THROW( message, ... )							return
#define HYSELECT_THROW_RETURNARG( message, arg, ... )			return arg
#define HYSELECT_CATCH( class_name, message )
#define HYSELECT_CATCH_ARG( class_name, message, arg, ... )
#endif


// To use in the same scope otherwise 'bOwner' will not be visible when calling 'HYSELECT_SETREDRAW_TRUE'.
// Sometimes we encapsulate a part of the code with 'SetRedraw( FALSE )' and 'SetRedraw( TRUE )' to avoid that Windows repaints controls while changing it.
// Between the two 'SetRedraw' calls, it can happen that we call one other method that itself also call 'SetRedraw( FALSE )' and 'SetRedraw( TRUE )'.
// In this case we don't want that 'SetRedraw( TRUE )' is applied because perhaps we have not yet finished to change the control.
// Windows API 'SetRedraw' simply send a 'WM_SETREDRAW' message. If parameter is 'FALSE', the API erases the 'WS_VISIBLE' style. If the parameter is 'TRUE',
// the API redraw the window and make it visible back. Thus we can play with that. 
// The first call to 'HYSELECT_SETREDRAW_FALSE' sets a local variable (Here 'bOwner') to 'true'.
// If the code below the first call to 'HYSELECT_SETREDRAW_FALSE' call one other method and that this one call also 'HYSELECT_SETREDRAW_FALSE', 
// the window is no more visible, and thus its own 'bOwner' stay to 'false'. When the 'HYSELECT_SETREDRAW_TRUE' in this method is called, and because 'bOwner' is set to 'false'
// we do nothing. We than go back in the first caller that calls 'HYSELECT_SETREDRAW_TRUE'. Here 'bOwner' is set to 'true' and then we can call 'HYSELECT_SETREDRAW_TRUE'.
#define HYSELECT_SETREDRAW_FALSE		bool bOwner = false; if( WS_VISIBLE == ( GetStyle() & WS_VISIBLE ) ) { bOwner = true; SetRedraw( FALSE); }
#define HYSELECT_SETREDRAW_TRUE			if( true == bOwner ) { SetRedraw( TRUE ); Invalidate(); UpdateWindow(); }

// These two macros allow to insert a text in a 'CListBox' and to scroll at the end.
#define INSERT_STRING_IN_PLIST_AND_SCROLL( List, String )	{ if( NULL != List ) { List->SetCurSel( List->InsertString( List->GetCount(), String ) ); } }
#define INSERT_STRING_IN_LIST_AND_SCROLL( List, String )	{ if( NULL != List ) { List.SetCurSel( List.InsertString( List.GetCount(), String ) ); } }
#define INSERT_STRING_IN_PLIST_NO_SCROLL( List, String )	{ if( NULL != List ) { List->InsertString( List->GetCount(), String ); } }
#define INSERT_STRING_IN_LIST_NO_SCROLL( List, String )	{ if( NULL != List ) { List.InsertString( List.GetCount(), String ); } }

#ifdef DEBUG
#define OUTPUT_DEBUG_STRING( str, ... )		{ CString temp; temp.Format( str, __VA_ARGS__ ); OutputDebugString( temp ); }
#define OUTPUT_DEBUG_STRINGA( str, ... )	{ CStringA temp; temp.Format( str, __VA_ARGS__ ); OutputDebugStringA( temp ); }
#else
#define OUTPUT_DEBUG_STRING( str, ... )
#define OUTPUT_DEBUG_STRINGA( str, ... )
#endif

///////////////////////////////////////////////////////
//			Other Constants
///////////////////////////////////////////////////////
#define CSTRING_LINE_SIZE_MAX		1024
#define RVSSEL_SELLEFT				0x01
#define RVSSEL_SELRIGHT				0x02
#define MAXMODULEDESCRIPTIONCHAR	64
#define ACTIVATIONWARNINGDAYS		30
#define AUTOSAVETIMER				1000*600			// 1000 msec * sec
#define CteEMPTY_STRING				_T("")

///////////////////////////////////////////////////////
//			CBI Limitations and constants
///////////////////////////////////////////////////////
#define CBIMAXSITENAMECHAR			20
#define	CBIMAXMODULENAMECHAR		10
#define CBIMAXVALVENAMECHAR			10
#define CBIKVVALVETYPE				_T("Kv-Mod")
#define CBICVVALVETYPE				_T("Cv-Mod")
#define CBIKVCVVALVESIZE			_T("..")
#define CBIVERSIONPREFIX			_T("PR")

///////////////////////////////////////////////////////
//			SCOPE Limitations and constants
///////////////////////////////////////////////////////
#define SCOPEMAXSITENAMECHAR		40
#define	SCOPEMAXMODULENAMECHAR		40

///////////////////////////////////////////////////////
//			FTP/HTTP Constants
///////////////////////////////////////////////////////
#define HYUPDATE					_T("HyUpdate")
#define AUTOUPDATE					_T("Auto Update")
#define LASTUPDATE					_T("Last Update")
#define SHOWNEWIN					_T("Show NewIn")
#define WEBSERVERDIR				_T("Web Server Directory")
#define GATEWAYUSED					_T("Gateway used")
#define FTPSERVER					_T("Ftp Server")
#define FTPLOGIN					_T("Ftp Login")
#define FTPSERVERPSW 				_T("Ftp Server Password")
#define FTPGATEWAY	 				_T("Ftp Gateway Name")
#define FTPGATEWAYPSW				_T("Ftp Gateway Password")
#define FTPPORT						_T("Ftp Port")
#define HTTPSERVER					_T("HTTP Server")
#define HTTPPORT					_T("HTTP Port")
#define HTTPLOGIN					_T("HTTP Login")
#define HTTPSERVERPSW				_T("HTTP Server Password")
#define HTTPGATEWAY					_T("HTTP Gateway Name")
#define HTTPGATEWAYPSW				_T("HTTP Gateway Password")

///////////////////////////////////////////////////////
//			Preferences
///////////////////////////////////////////////////////
#define PREFERENCE					_T("Preference")
#define AUTOSAVING					_T("Auto Saving Actif")
#define AUTOSAVINGEXTENSION			_T("-autosave")

///////////////////////////////////////////////////////
//			Window ID for saving column width in individual selection
///////////////////////////////////////////////////////
#define CW_WINDOWID_INDSELFIRST					0
#define CW_WINDOWID_INDSELBCV					CW_WINDOWID_INDSELFIRST		// For the BCV individual selection (DlgIndSelBCV).
#define CW_WINDOWID_INDSELBV					1							// For the BV individual selection (DlgIndSelBv).
#define CW_WINDOWID_INDSELCV					2							// For the CV individual selection (DlgIndSelCV).
#define CW_WINDOWID_INDSELDPC					3							// For the DpC individual selection (DlgIndSelDpC).
#define CW_WINDOWID_INDSELPICV					4							// For the PICV individual selection (DlgIndSelPICV).
#define CW_WINDOWID_INDSELTRV					5							// For the Trv individual selection (DlgIndSelTrv).
#define CW_WINDOWID_INDSELSEPARATOR				6							// For the Separator individual selection (DlgIndSelSeparator).
#define CW_WINDOWID_INDSELPRESSUREMAINTENANCE	7							// For the Pressure maintenance individual selection (DlgIndSelPressureMaintenance).
#define CW_WINDOWID_INDSELSV					8							// For the ShutoffValve individual selection (DlgIndSelSV).
#define CW_WINDOWID_INDSELDPCBCV				9							// For the Combined Dp controller, control and balancing valve individual selection (DlgIndSelDpCBCV).
#define CW_WINDOWID_INDSELSAFETYVALVE			10							// For the safety valve individual selection (DlgIndSelSafetyValve).
#define CW_WINDOWID_INDSEL6WAYVALVE				11							// For the 6-way valve selection (DlgIndSel6WayValve).
#define CW_WINDOWID_INDSELSMARTCONTROLVALVE		12							// For the smart control valve selection (DlgIndSelSmartControlValve).
#define CW_WINDOWID_INDSELSMARTDPC		13							// For the smart differential pressure controller selection (DlgIndSelSmartDpC).
#define CW_WINDOWID_INDSELLAST					14

///////////////////////////////////////////////////////
//			Window ID for saving column width in batch selection
///////////////////////////////////////////////////////
#define CW_WINDOWID_BATCHSELBCV				0							// For the BCV batch selection (DlgBatchSelBCV).
#define CW_WINDOWID_BATCHSELBV				1							// For the BV batch selection (DlgBatchSelBv).
#define CW_WINDOWID_BATCHSELCV				2							// For the CV batch selection (DlgBatchSelCV).
#define CW_WINDOWID_BATCHSELDPC				3							// For the DpC batch selection (DlgBatchSelDpC).
#define CW_WINDOWID_BATCHSELPICV			4							// For the PICV batch selection (DlgBatchSelPICV).
#define CW_WINDOWID_BATCHSELTRV				5							// For the Trv batch selection (DlgBatchSelTrv).
#define CW_WINDOWID_BATCHSELSEPARATOR		6							// For the Separator batch selection (DlgBatchSelSeparator).
#define CW_WINDOWID_BATCHSMARTCONTROLVALVE	7							// For the smart control valve batch selection (DlgBatchSelSmartControlValve).
#define CW_WINDOWID_BATCHSMARTDPC			8							// For the smart differential pressure controller batch selection (CDlgBatchSelSmartDpC).

///////////////////////////////////////////////////////
//			Keyboard shortcuts
///////////////////////////////////////////////////////
#define KS_CONTROL_C					1
#define KS_CONTROL_X					2
#define KS_CONTROL_V					3
#define KS_CONTROL_A					4

///////////////////////////////////////////////////////
//			Registry section names
///////////////////////////////////////////////////////
#define REGISTRYSECTIONNAME_DLGTECHPARAM		_T("DialogTechParam")
#define REGISTRYSECTIONNAME_PIPESERIES			_T("Pipe Series")

///////////////////////////////////////////////////////
//			Registry key names
///////////////////////////////////////////////////////
#define REGISTRYKEYNAME_VALVEIMAGETYPE			_T("Valve image type")

///////////////////////////////////////////////////////
//			Structures
///////////////////////////////////////////////////////
struct RadInfo_struct
{
	double dFlow;						// Flow
	bool   bAvailableDpChecked;			// True if the available Dp is known/given
	double dAvailableDp;				// Available differential pressure
	double dSupplyTemperature;			// Supply temperature
	double dReturnTemperature;			// Return temperature
	double dRoomTemperature;			// Room temperature
	double dRequiredHeatOutput;			// Required heat output
	double dMinPower;					// Minimum power to be installed
	double dRecommendedPower;			// Recommended power to be installed
	double dInstalledPower;				// Installed power at NC
	double dMinDT;						// Minimum DT between supply and return
	double dRequiredDT;					// Required DT between supply and return
	double dNCSupplyT;					// NC Supply temperature
	double dNCReturnT;					// NC Return temperature
	double dNCRoomT;					// NC Room temperature
	struct RadInfo_struct()
	{
		Reset();
	}
	void Reset()
	{
		dFlow = 0.0;
		bAvailableDpChecked = false;
		dAvailableDp = 0.0;
		dSupplyTemperature = 0.0;
		dReturnTemperature = 0.0;
		dRoomTemperature = 0.0;
		dRequiredHeatOutput = 0.0;
		dMinPower = 0.0;
		dRecommendedPower = 0.0;
		dInstalledPower = 0.0;
		dMinDT = 0.0;
		dRequiredDT = 0.0;
		dNCSupplyT = 0.0;
		dNCReturnT = 0.0;
		dNCRoomT = 0.0;
	}
};

///////////////////////////////////////////////////////
//			Enums
///////////////////////////////////////////////////////
enum ProjectType
{
	InvalidProjectType = -1,
	Heating = 1, 
	Cooling = 2,
	Solar = 4,
	ChangeOver = 8,
	All = ( Heating | Cooling | Solar ) // Pressurisation use only
};

enum PressurisationNorm
{
	PN_Undefined,
	PN_None,
	PN_EN12828,
	PN_EN12953,
	PN_SWKIHE301_01,
	PN_DM11275
};

enum HeatGeneratorType
{
	HGT_Undefined,
	HGT_DirectlyHeated,
	HGT_IndirectlyHeated
};

// 3 State bool
typedef enum eBool3
{
	eb3Undef = -1,
	eb3False = 0,
	eb3True = 1
};

// Different selection mode used with 6-way valve.
typedef enum e6WayValveSelectionMode
{
	e6Way_Undefined = -1,
	e6Way_Alone,
	e6Way_EQMControl,							// Proportional control with PIBCV (With TA-Modulator and TA-Compact-P).
	e6Way_OnOffControlWithPIBCV,				// On/Off control with PIBCV (TA-Compact-P).
	e6Way_OnOffControlWithSTAD,					// On/Off with STAD.
	e6Way_Last = e6Way_OnOffControlWithSTAD
};

enum SideDefinition
{
	Undefined		= -1,
	BothSide		= 0,
	CoolingSide		= 1,
	HeatingSide		= 2,
	LastSide		= 3
};

// For smart control valve.
enum SmartValveControlMode
{
	SCVCM_Undefined = -1,
	SCVCM_Flow,
	SCVCM_Power,
	SCVCM_Position
};


#endif // !defined(GLOBAL_H__INCLUDED_)

