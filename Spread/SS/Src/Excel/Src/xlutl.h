/*********************************************************
* XLutl.h
*
* Copyright (C) 1999 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------
*	RAP	- 11.19.98
*********************************************************/

// LogFile Constants
//Import-Export
#define LOG_OPEN                    100
#define LOG_CLOSE                   101
#define LOG_REGISTRYPROPS           102
//Import
#define LOG_LOADFILE                120
#define LOG_INTEGERCELLTYPE         121
#define LOG_INVALIDTOKEN            122
#define LOG_FORMATNOTSUPPORTED      123
#define LOG_DATEFORMATNOTSUPPORTED  124
#define LOG_FORMAT0IGNORED          125
#define LOG_FORMAT0FORCED           126
#define LOG_IGNOREDTOKEN            127
#define LOG_FORMATFRACTION          128
#define LOG_FORMATPERCENT           129
#define LOG_FORMATSCIENTIFIC        130
#define LOG_PMTFUNCTIONLOAD         131
#define LOG_CELLOBJECTSNOTSUPPORTED 132
#define LOG_FORMPTGNOTSUPPORTED     133
#define LOG_FUNCFORMCELLTYPE        134
#define LOG_FUNCNOTSUPPORTED        135
#define LOG_FORMERROR               136
#define LOG_FLOATOUTOFBOUNDS        137
#define LOG_CURRENCYMULTIPLECHARS   138
#define LOG_CURRENCYNOTFIRST        139
#define LOG_RICHTEXTNOTSUPPORTED    140
#define LOG_IFFUNCTIONLOAD          141
#define LOG_TRUNCFUNCTIONLOAD       142
#define LOG_FEB291900               143
#define LOG_PASSWORD                144
//Export
#define LOG_SAVEFILE                160
#define LOG_DATEOUTOFRANGE          161
#define LOG_OBJCELLTYPE             162
#define LOG_PMTFUNCTIONSAVE         163
#define LOG_PICTOKENNOTSUPPORTED    164
#define LOG_TOOMANYCOLS             165
#define LOG_TOOMANYROWS             166
#define LOG_CUSTFUNCNOTSUPPORTED    167
#define LOG_XROOTFUNCTIONSAVE       168
#define LOG_IFFUNCTIONSAVE          169
#define LOG_EXPORTCHECKBOX          170
#define LOG_EXPORTBUTTON            171
#define LOG_EXPORTCOMBOBOX          172
#define LOG_CUSTHEADERNOTSUPPORTED  173
#define LOG_URLFUNCTIONNOTIFY       174
#define LOG_EXPORTSTATICTEXT        175
//Spread30 Only
#define LOG_MULTIPLEFORMATS         190

// LogFile Strings
// Import/Export strings
/*
#define LOGS_REGISTRYPROPS "Excel reads the AllowDragDrop, AutoClipboard, and EditEnterAction properties from the registry. If you modify these properties in Spread, the changes are not saved at export."
#define LOGS_FP "===== FarPoint Technologies, Inc ====="

// Import strings
#define LOGS_LOADFILE "Import Excel File: "
#define LOGS_CLOSE_IMPWB "===== End of Workbook General Load ====="
#define LOGS_CLOSE_IMPSHEET "===== End of Sheet Load ====="
#define LOGS_INTEGERCELLTYPE "Rounding/Truncating difference: If a float value is placed in an integer cell, Spread truncates the value.  Excel rounds float values in integer cells. Value from Excel = "
#define LOGS_INVALIDTOKEN "Spread does not support the format token. Excel token = "
#define LOGS_FORMATNOTSUPPORTED "Spread does not support this format. Excel format = "
#define LOGS_DATEFORMATNOTSUPPORTED "Spread does not support this date format. Excel format = "
#define LOGS_FORMAT0IGNORED "Spread ignored zeros in format w/ leading zeros. Excel format = "
#define LOGS_FORMAT0FORCED "Spread substituted zeros in format w/ leading or trailing non-zeros. Excel format = "
#define LOGS_IGNOREDTOKEN "Spread recognizes this format token, but does not support it. Excel token = "
#define LOGS_FORMATFRACTION "Spread does not support fraction formats; the value has been changed to a decimal representation. Excel format = "
#define LOGS_FORMATPERCENT "Spread does not support percent formats; the value has been changed to a decimal representation. Excel format = "
#define LOGS_FORMATSCIENTIFIC "Spread does not support scientific notation; the value has been changed to a decimal representation. Excel format = "
#define LOGS_PMTFUNCTIONLOAD "PMT() Function - TermsPerYear argument defaults to 12 when Spread imports the Excel file."
#define LOGS_TRUNCFUNCTIONLOAD "TRUNCATE() Function - Precision argument defaults to 0 when Spread imports the Excel file."
#define LOGS_IFFUNCTIONLOAD "The IF() function is not imported into Spread. An empty cell has been created in the Spread."
#define LOGS_CELLOBJECTSNOTSUPPORTED "Spread does not support graphic objects. The graphic object will not be represented in Spread."
#define LOGS_FORMPTGNOTSUPPORTED "Import encountered unrecognized Excel formula format tag. An empty cell has been created in Spread. Excel format tag = "
#define LOGS_FUNCFORMCELLTYPE "Import encountered a function or formula in the Excel file."
#define LOGS_FUNCNOTSUPPORTED "Spread does not support the specified function. An empty cell has been created in the Spread file. Excel function = "
#define LOGS_FORMERROR "Import encountered a formula error reported from Excel. An empty cell has been created in the Spread file."
#define LOGS_FLOATOUTOFBOUNDS "Float value in Excel is beyond the range of Spread's float cell type. An empty cell has been created in Spread. Excel float value = "
#define LOGS_CURRENCYMULTIPLECHARS "Excel’s currency symbol uses multiple characters. Spread does not support multiple characters; therefore, Spread uses the first character of the Excel symbol. Excel symbol ="
#define LOGS_CURRENCYNOTFIRST "Excel displayed the currency symbol somewhere other than to the left of the value. Spread does not support this; therefore, Spread placed the symbol to the left of the value. Excel format ="
#define LOGS_RICHTEXTNOTSUPPORTED "Spread does not support Rich Text Formats. The text is displayed in the first format encountered. String ="
// Export strings
#define LOGS_SAVEFILE "Export Excel File: "
#define LOGS_CLOSE_EXPORT "===== End of Export ====="
#define LOGS_DATEOUTOFRANGE "Excel dates must fall between 1/1/1900 and 12/31/9999. The date in Spread is not in that range, and has not been saved at export. Date in Spread = "
#define LOGS_OBJCELLTYPE "Excel does not support the specified Spread cell type; an empty cell has been created in the Excel file. Spread cell type = "
#define LOGS_PMTFUNCTIONSAVE "The PMT() function is not exported to Excel. An empty cell has been created in the Excel file."
#define LOGS_XROOTFUNCTIONSAVE "The XROOT() function is not exported to Excel. An empty cell has been created in the Excel file."
#define LOGS_IFFUNCTIONSAVE "The IF() function is not exported to Excel. An empty cell has been created in the Excel file."
#define LOGS_PICTOKENNOTSUPPORTED "Excel does not support the specified mask definition character. This mask character has not been exported to Excel. Spread mask character = "
#define LOGS_TOOMANYCOLS "The Spread control contains more columns than Excel supports. Excel supports only 256 columns; columns beyond 256 have been truncated. Number of columns in Spread = "
#define LOGS_TOOMANYROWS "The Spread control contains more rows than Excel supports. Excel supports only 65536 rows; rows beyond 65536 have been truncated. Number of rows in Spread = "
#define LOGS_CUSTFUNCNOTSUPPORTED "Custom functions are not exported to Excel."
//Spread30 Only
#define LOGS_MULTIPLEFORMATS "Spread 3.0 does not support multiple format sections. Spread will represent the value using the default Spread format. Format in Excel = "
*/
