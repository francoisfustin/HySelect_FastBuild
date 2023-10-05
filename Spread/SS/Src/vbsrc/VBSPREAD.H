/*************
* VBSPREAD.H
*************/

#ifndef VBSPREAD_H
#define VBSPREAD_H

#include "..\sssrc\spread\spread.h"

#ifdef SS_SPREADVBX
#include "..\vbx\vbss_def.h"
#endif

//DBOCX Change to many functions: "HCTL hCtl" to "LONG lObject"
//Since this affects so many functions I didn't mark them.

#define ERR_RANGE_COL    30500
#define ERR_RANGE_ROW    30501

#define ERRMSG_RANGE_COL "Invalid column value"
#define ERRMSG_RANGE_ROW "Invalid row value"

#define CLASS_POPUP             "SpreadPopup"
#define SM_SHOWPOPUP            WM_USER + 0 
#define SDM_HEREIAM             WM_USER + 112 // Spread Designer initial notification.
#define SDM_GETCOLS       WM_USER + 111 // Get the DataField names for data binding.
#define SDM_EXIT        WM_USER + 110 // Spread Designer is exitting...

#define EDITERROR_CELLLOCKED       1
#define EDITERROR_COLRESTRICTED    2
#define EDITERROR_ROWRESTRICTED    3
#define EDITERROR_COLROWRESTRICTED 4
#define EDITERROR_INVALIDDATA      5
#define EDITERROR_INVALIDPASTEDATA 6
#define EDITERROR_TYPINGPROHIBITED 7
#define EDITERROR_EDITMODEINS      8
#define EDITERROR_CHECKBOXTYPING   9
#define EDITERROR_INVALIDCOLTYPE   10

typedef struct DataOptions
   {
   unsigned fAutoSave : 1;
   unsigned fHeadings : 1;
   unsigned fCellTypes : 1;
   unsigned fAutoFill : 1;
   unsigned fSizeCols : 2;
   unsigned fAutoDataInform : 1;
   } DATAOPTS, FAR *LPDATAOPTS;

typedef struct DBInfo
   {
   BOOL           fConnected;
   long           hStmt;
   long           hDB;
   TBGLOBALHANDLE hDataConnect;
   TBGLOBALHANDLE hDataSelect;
   } DBINFO, FAR *LPDBINFO;

#ifndef SSP_NO_BORDERSTYLEON
#define IPROP_SPREAD_BORDERSTYLEON       0
#endif
#ifndef SSP_NO_CTLNAME
#define IPROP_SPREAD_CTLNAME             1
#endif
#ifndef SSP_NO_DRAGICON
#define IPROP_SPREAD_DRAGICON            2
#endif
#ifndef SSP_NO_DRAGMODE
#define IPROP_SPREAD_DRAGMODE            3
#endif
#ifndef SSP_NO_ENABLED
#define IPROP_SPREAD_ENABLED             4
#endif
#ifndef SSP_NO_FONTBOLD
#define IPROP_SPREAD_FONTBOLD            5
#endif
#ifndef SSP_NO_FONTITALIC
#define IPROP_SPREAD_FONTITALIC          6
#endif
#ifndef SSP_NO_FONTNAME
#define IPROP_SPREAD_FONTNAME            7
#endif
#ifndef SSP_NO_FONTSIZE
#define IPROP_SPREAD_FONTSIZE            8
#endif
#ifndef SSP_NO_FONTSTRIKE
#define IPROP_SPREAD_FONTSTRIKE          9
#endif
#ifndef SSP_NO_FONTUNDER
#define IPROP_SPREAD_FONTUNDER           10
#endif
#ifndef SSP_NO_HEIGHT
#define IPROP_SPREAD_HEIGHT              11
#endif
#ifndef SSP_NO_INDEX
#define IPROP_SPREAD_INDEX               12
#endif
#ifndef SSP_NO_LEFT
#define IPROP_SPREAD_LEFT                13
#endif
#ifndef SSP_NO_PARENT
#define IPROP_SPREAD_PARENT              14
#endif
#ifndef SSP_NO_TABINDEX
#define IPROP_SPREAD_TABINDEX            15
#endif
#ifndef SSP_NO_TABSTOP
#define IPROP_SPREAD_TABSTOP             16
#endif
#ifndef SSP_NO_TAG
#define IPROP_SPREAD_TAG                 17
#endif
#ifndef SSP_NO_TOP
#define IPROP_SPREAD_TOP                 18
#endif
#ifndef SSP_NO_VISIBLE
#define IPROP_SPREAD_VISIBLE             19
#endif
#ifndef SSP_NO_WIDTH
#define IPROP_SPREAD_WIDTH               20
#endif
#ifndef SSP_NO_ACTION
#define IPROP_SPREAD_ACTION              21
#endif
#ifndef SSP_NO_ACTIVECELLCOL
#define IPROP_SPREAD_ACTIVECELLCOL       22
#endif
#ifndef SSP_NO_ACTIVECELLROW
#define IPROP_SPREAD_ACTIVECELLROW       23
#endif
#ifndef SSP_NO_PRINTORIENTATION
#define IPROP_SPREAD_PRINTORIENTATION    24
#endif
#ifndef SS_V30
#ifndef SSP_NO_ALLOWSELECTBLOCK
#define IPROP_SPREAD_ALLOWSELECTBLOCK    25
#endif
#else
#ifndef SSP_NO_APPEARANCE
#define IPROP_SPREAD_APPEARANCE          25
#endif
#endif
#ifndef SSP_NO_AUTOCALC
#define IPROP_SPREAD_AUTOCALC            26
#endif
#ifndef SSP_NO_BACKCOLOR
#define IPROP_SPREAD_BACKCOLOR           27
#endif
#ifndef SS_V30
#ifndef SSP_NO_CALCDEPENDENCIES
#define IPROP_SPREAD_CALCDEPENDENCIES    28
#endif
#else
#ifndef SSP_NO_TYPECOMBOBOXWIDTH
#define IPROP_SPREAD_TYPECOMBOWIDTH      28
#endif
#endif
#ifndef SSP_NO_CELLTYPE
#define IPROP_SPREAD_CELLTYPE            29
#endif
#ifndef SSP_NO_CLIP
#define IPROP_SPREAD_CLIP                30
#endif
#ifndef SSP_NO_COL
#define IPROP_SPREAD_COL                 31
#endif
#ifndef SSP_NO_COL2
#define IPROP_SPREAD_COL2                32
#endif
#ifndef SSP_NO_COLWIDTH
#define IPROP_SPREAD_COLWIDTH            33
#endif
#ifndef SSP_NO_DISPLAYCOLHDRS
#define IPROP_SPREAD_DISPLAYCOLHDRS      34
#endif
#ifndef SSP_NO_DISPLAYROWHDRS
#define IPROP_SPREAD_DISPLAYROWHDRS      35
#endif
#ifndef SSP_NO_EDITMODE
#define IPROP_SPREAD_EDITMODE            36
#endif
#ifndef SSP_NO_FILENUM
#define IPROP_SPREAD_FILENUM             37
#endif
#ifndef SSP_NO_FORECOLOR
#define IPROP_SPREAD_FORECOLOR           38
#endif
#ifndef SSP_NO_FORMULA
#define IPROP_SPREAD_FORMULA             39
#endif
#ifndef SS_V30
#ifndef SSP_NO_GRIDLINES
#define IPROP_SPREAD_GRIDLINES           40
#endif
#else
#ifndef SSP_NO_TYPECOMBOBOXMAXDROP
#define IPROP_SPREAD_TYPECOMBOMAXDROP    40
#endif
#endif
#ifndef SSP_NO_LEFTCOL
#define IPROP_SPREAD_LEFTCOL             41
#endif
#ifndef SSP_NO_LOCK
#define IPROP_SPREAD_LOCK                42
#endif
#ifndef SSP_NO_MAXCOLS
#define IPROP_SPREAD_MAXCOLS             43
#endif
#ifndef SSP_NO_MAXROWS
#define IPROP_SPREAD_MAXROWS             44
#endif
#ifndef SSP_NO_POSITION
#define IPROP_SPREAD_POSITION            45
#endif
#ifndef SSP_NO_PROTECT
#define IPROP_SPREAD_PROTECT             46
#endif
#ifndef SSP_NO_REDRAW
#define IPROP_SPREAD_REDRAW              47
#endif
#ifndef SSP_NO_RESTRICTCOLS
#define IPROP_SPREAD_RESTRICTCOLS        48
#endif
#ifndef SSP_NO_RESTRICTROWS
#define IPROP_SPREAD_RESTRICTROWS        49
#endif
#ifndef SSP_NO_ROW
#define IPROP_SPREAD_ROW                 50
#endif
#ifndef SSP_NO_ROW2
#define IPROP_SPREAD_ROW2                51
#endif
#ifndef SSP_NO_ROWHEIGHT
#define IPROP_SPREAD_ROWHEIGHT           52
#endif
#ifndef SSP_NO_SELECTBLOCKCOL
#define IPROP_SPREAD_SELECTBLOCKCOL      53
#endif
#ifndef SSP_NO_SELECTBLOCKCOL2
#define IPROP_SPREAD_SELECTBLOCKCOL2     54
#endif
#ifndef SSP_NO_SELECTBLOCKROW
#define IPROP_SPREAD_SELECTBLOCKROW      55
#endif
#ifndef SSP_NO_SELECTBLOCKROW2
#define IPROP_SPREAD_SELECTBLOCKROW2     56
#endif
#ifndef SSP_NO_SELLENGTH
#define IPROP_SPREAD_SELLENGTH           57
#endif
#ifndef SSP_NO_SELSTART
#define IPROP_SPREAD_SELSTART            58
#endif
#ifndef SSP_NO_SELTEXT
#define IPROP_SPREAD_SELTEXT             59
#endif
#ifndef SSP_NO_SHADOWCOLOR
#define IPROP_SPREAD_SHADOWCOLOR         60
#endif
#ifndef SSP_NO_SHADOWDARK
#define IPROP_SPREAD_SHADOWDARK          61
#endif
#ifndef SSP_NO_SHADOWTEXT
#define IPROP_SPREAD_SHADOWTEXT          62
#endif
#ifndef SSP_NO_TEXT
#define IPROP_SPREAD_TEXT                63
#endif
#ifndef SSP_NO_TOPROW
#define IPROP_SPREAD_TOPROW              64
#endif
#ifndef SSP_NO_DATECENTURY
#define IPROP_SPREAD_DATECENTURY         65
#endif
#ifndef SSP_NO_DATEFORMAT
#define IPROP_SPREAD_DATEFORMAT          66
#endif
#ifndef SSP_NO_DATEMAX
#define IPROP_SPREAD_DATEMAX             67
#endif
#ifndef SSP_NO_DATEMIN
#define IPROP_SPREAD_DATEMIN             68
#endif
#ifndef SSP_NO_DATESEPARATOR
#define IPROP_SPREAD_DATESEPARATOR       69
#endif
#ifndef SS_V30
#ifndef SSP_NO_EDITCASE
#define IPROP_SPREAD_EDITCASE            70
#endif
#else
#ifndef SSP_NO_TYPEVALIGN
#define IPROP_SPREAD_TYPEVALIGN          70
#endif
#endif
#ifndef SSP_NO_EDITCHARSET
#define IPROP_SPREAD_EDITCHARSET         71
#endif
#ifdef SS_V30
#ifndef SSP_NO_TEXTTIP
#define IPROP_SPREAD_TEXTTIP             72
#endif
#else
#ifndef SSP_NO_EDITJUSTIFY
#define IPROP_SPREAD_EDITJUSTIFY         72
#endif
#endif
#ifndef SSP_NO_EDITLEN
#define IPROP_SPREAD_EDITLEN             73
#endif
#ifndef SSP_NO_FLOATDECPLACES
#define IPROP_SPREAD_FLOATDECPLACES      74
#endif
#ifndef SSP_NO_FLOATMAX
#define IPROP_SPREAD_FLOATMAX            75
#endif
#ifndef SSP_NO_FLOATMIN
#define IPROP_SPREAD_FLOATMIN            76
#endif
#ifndef SSP_NO_FLOATMONEY
#define IPROP_SPREAD_FLOATMONEY          77
#endif
#ifndef SSP_NO_FLOATSEPARATOR
#define IPROP_SPREAD_FLOATSEPARATOR      78
#endif
#ifndef SSP_NO_INTEGERMAX
#define IPROP_SPREAD_INTEGERMAX          79
#endif
#ifndef SSP_NO_INTEGERMIN
#define IPROP_SPREAD_INTEGERMIN          80
#endif
#ifndef SSP_NO_PICMASK
#define IPROP_SPREAD_PICMASK             81
#endif
#ifdef SS_V30
#ifndef SSP_NO_TEXTTIPDELAY
#define IPROP_SPREAD_TEXTTIPDELAY        82
#endif
#else
#ifndef SSP_NO_TEXTJUSTIFYHORIZ
#define IPROP_SPREAD_TEXTJUSTIFYHORIZ    82
#endif
#endif
#ifndef SSP_NO_TEXTJUSTIFYVERT
#define IPROP_SPREAD_TEXTJUSTIFYVERT     83
#endif
#ifndef SSP_NO_TEXTSHADOW
#define IPROP_SPREAD_TEXTSHADOW          84
#endif
#ifndef SSP_NO_TEXTWORDWRAP
#define IPROP_SPREAD_TEXTWORDWRAP        85
#endif
#ifndef SSP_NO_TIMEMAX
#define IPROP_SPREAD_TIMEMAX             86
#endif
#ifndef SSP_NO_TIMEMIN
#define IPROP_SPREAD_TIMEMIN             87
#endif
#ifndef SSP_NO_TIMESECONDS
#define IPROP_SPREAD_TIMESECONDS         88
#endif
#ifndef SSP_NO_TIMESEPARATOR
#define IPROP_SPREAD_TIMESEPARATOR       89
#endif
#ifndef SSP_NO_TIME24HOUR
#define IPROP_SPREAD_TIME24HOUR          90
#endif
#ifndef SSP_NO_BLOCKMODE
#define IPROP_SPREAD_BLOCKMODE           91
#endif
#ifndef SSP_NO_SCROLLBARS
#define IPROP_SPREAD_SCROLLBARS          92
#endif
#ifndef SSP_NO_ISBLOCKSELECTED
#define IPROP_SPREAD_ISBLOCKSELECTED     93
#endif
#ifndef SSP_NO_SPREADDESIGNER
#define IPROP_SPREAD_SPREADDESIGNER      94
#endif
#ifndef SSP_NO_VALUE
#define IPROP_SPREAD_VALUE               95
#endif
#ifndef SSP_NO_PICDEFTEXT
#define IPROP_SPREAD_PICDEFTEXT          96
#endif
#ifndef SSP_NO_EDITMODEPERMANENT
#define IPROP_SPREAD_EDITMODEPERMANENT   97
#endif
#ifndef SSP_NO_AUTOSIZE
#define IPROP_SPREAD_AUTOSIZE            98
#endif
#ifndef SSP_NO_VISIBLEROWS
#define IPROP_SPREAD_VISIBLEROWS         99
#endif
#ifndef SSP_NO_VISIBLECOLS
#define IPROP_SPREAD_VISIBLECOLS         100
#endif
#ifndef SSP_NO_ROWSFROZEN
#define IPROP_SPREAD_ROWSFROZEN          101
#endif
#ifndef SSP_NO_COLSFROZEN
#define IPROP_SPREAD_COLSFROZEN          102
#endif
#ifndef SSP_NO_SCROLLBAREXTMODE
#define IPROP_SPREAD_SCROLLBAREXTMODE    103
#endif
#ifndef SSP_NO_BUTTONALIGN
#define IPROP_SPREAD_BUTTONALIGN         104
#endif
#ifndef SSP_NO_BUTTONTEXT
#define IPROP_SPREAD_BUTTONTEXT          105
#endif
#ifndef SSP_NO_BUTTONPICTNAME
#define IPROP_SPREAD_BUTTONPICTNAME      106
#endif
#ifndef SSP_NO_BUTTONPICTDOWN
#define IPROP_SPREAD_BUTTONPICTDOWN      107
#endif
#ifndef SSP_NO_BUTTONTYPE
#define IPROP_SPREAD_BUTTONTYPE          108
#endif
#ifndef SSP_NO_BUTTONCOLOR
#define IPROP_SPREAD_BUTTONCOLOR         109
#endif
#ifndef SSP_NO_BUTTONBORDERCOLOR
#define IPROP_SPREAD_BUTTONBORDERCOLOR   110
#endif
#ifndef SSP_NO_BUTTONDARKCOLOR
#define IPROP_SPREAD_BUTTONDARKCOLOR     111
#endif
#ifndef SSP_NO_BUTTONLIGHTCOLOR
#define IPROP_SPREAD_BUTTONLIGHTCOLOR    112
#endif
#ifndef SSP_NO_BUTTONTEXTCOLOR
#define IPROP_SPREAD_BUTTONTEXTCOLOR     113
#endif
#ifndef SSP_NO_PROCESSTAB
#define IPROP_SPREAD_PROCESSTAB          114
#endif
#ifndef SSP_NO_BUTTONSHADOWSIZE
#define IPROP_SPREAD_BUTTONSHADOWSIZE    115
#endif
#ifndef SSP_NO_SCROLLBARSHOWMAX
#define IPROP_SPREAD_SCROLLBARSHOWMAX    116
#endif
#ifndef SSP_NO_SCROLLBARMAXALIGN
#define IPROP_SPREAD_SCROLLBARMAXALIGN   117
#endif
#ifndef SSP_NO_LOCKBACKCOLOR
#define IPROP_SPREAD_LOCKBACKCOLOR       118
#endif
#ifndef SSP_NO_LOCKFORECOLOR
#define IPROP_SPREAD_LOCKFORECOLOR       119
#endif
#ifndef SSP_NO_EDITPASSWORD
#define IPROP_SPREAD_EDITPASSWORD        120
#endif
#ifndef SSP_NO_COMBOLIST
#define IPROP_SPREAD_COMBOLIST           121
#endif
#ifndef SSP_NO_PRINTJOBNAME
#define IPROP_SPREAD_PRINTJOBNAME        122
#endif
#ifndef SSP_NO_PRINTABORTMSG
#define IPROP_SPREAD_PRINTABORTMSG       123
#endif
#ifndef SSP_NO_PRINTHEADER
#define IPROP_SPREAD_PRINTHEADER         124
#endif
#ifndef SSP_NO_PRINTFOOTER
#define IPROP_SPREAD_PRINTFOOTER         125
#endif
#ifndef SSP_NO_PRINTBORDER
#define IPROP_SPREAD_PRINTBORDER         126
#endif
#ifndef SSP_NO_PRINTSHADOWS
#define IPROP_SPREAD_PRINTSHADOWS        127
#endif
#ifndef SSP_NO_PRINTGRID
#define IPROP_SPREAD_PRINTGRID           128
#endif
#ifndef SSP_NO_PRINTCOLHEADERS
#define IPROP_SPREAD_PRINTCOLHEADERS     129
#endif
#ifndef SSP_NO_PRINTROWHEADERS
#define IPROP_SPREAD_PRINTROWHEADERS     130
#endif
#ifndef SSP_NO_PRINTUSEDATAMAX
#define IPROP_SPREAD_PRINTUSEDATAMAX     131
#endif
#ifndef SSP_NO_PRINTTYPE
#define IPROP_SPREAD_PRINTTYPE           132
#endif
#ifndef SSP_NO_PRINTPAGESTART
#define IPROP_SPREAD_PRINTPAGESTART      133
#endif
#ifndef SSP_NO_PRINTPAGEEND
#define IPROP_SPREAD_PRINTPAGEEND        134
#endif
#ifndef SSP_NO_PRINTMARGINLEFT
#define IPROP_SPREAD_PRINTMARGINLEFT     135
#endif
#ifndef SSP_NO_PRINTMARGINTOP
#define IPROP_SPREAD_PRINTMARGINTOP      136
#endif
#ifndef SSP_NO_PRINTMARGINRIGHT
#define IPROP_SPREAD_PRINTMARGINRIGHT    137
#endif
#ifndef SSP_NO_PRINTMARGINBOTTOM
#define IPROP_SPREAD_PRINTMARGINBOTTOM   138
#endif
#ifndef SSP_NO_UNITTYPE
#define IPROP_SPREAD_UNITTYPE            139
#endif
#ifndef SSP_NO_EDITENTERACTION
#define IPROP_SPREAD_EDITENTERACTION     140
#endif
#ifndef SSP_NO_GRAYAREABACKCOLOR
#define IPROP_SPREAD_GRAYAREABACKCOLOR   141
#endif

#ifdef SS_V30
#ifndef SSP_NO_SCROLLBARTRACK
#define IPROP_SPREAD_SCROLLBARTRACK      142
#endif
#else
#ifndef SSP_NO_GRAYAREAFORECOLOR
#define IPROP_SPREAD_GRAYAREAFORECOLOR   142
#endif
#endif

#ifndef SSP_NO_ARROWSEXITEDITMOD
#define IPROP_SPREAD_ARROWSEXITEDITMOD   143
#endif
#ifndef SSP_NO_MOVEACTIVEONFOCUS
#define IPROP_SPREAD_MOVEACTIVEONFOCUS   144
#endif
#ifndef SSP_NO_TYPEPICTPICTURE
#define IPROP_SPREAD_TYPEPICTPICTURE     145
#endif
#ifndef SSP_NO_TYPEPICTCENTER
#define IPROP_SPREAD_TYPEPICTCENTER      146
#endif
#ifndef SSP_NO_TYPEPICTSTRETCH
#define IPROP_SPREAD_TYPEPICTSTRETCH     147
#endif
#ifndef SSP_NO_TYPEPICTSCALE
#define IPROP_SPREAD_TYPEPICTSCALE       148
#endif
#ifndef SSP_NO_RETAINSELBLOCK
#define IPROP_SPREAD_RETAINSELBLOCK      149
#endif
#ifndef SSP_NO_ALLOWUSERFORMULAS
#define IPROP_SPREAD_ALLOWUSERFORMULAS   150
#endif
#ifndef SSP_NO_NOBEEP
#define IPROP_SPREAD_NOBEEP              151
#endif
#ifndef SSP_NO_PRINTCOLOR
#define IPROP_SPREAD_PRINTCOLOR          152
#endif
#ifndef SSP_NO_OPERATIONMODE
#define IPROP_SPREAD_OPERATIONMODE       153
#endif
#ifndef SSP_NO_EDITCHARCASE
#define IPROP_SPREAD_EDITCHARCASE        154
#endif

#ifndef SS_V30
#ifndef SSP_NO_DATACONNECT
#define IPROP_SPREAD_DATACONNECT         155
#endif
#else
#ifndef SSP_NO_CLIPBOARDOPTIONS
#define IPROP_SPREAD_CLIPBOARDOPTIONS    155
#endif
#endif

#ifndef SSP_NO_EDITMODEREPLACE
#define IPROP_SPREAD_EDITMODEREPLACE     156
#endif
#ifndef SSP_NO_CLIPVALUE
#define IPROP_SPREAD_CLIPVALUE           157
#endif
#ifndef SSP_NO_EDITMULTILINE
#define IPROP_SPREAD_EDITMULTILINE       158
#endif
#ifndef SSP_NO_HWND
#define IPROP_SPREAD_HWND                159
#endif

#ifndef SS_V30
#ifndef SSP_NO_LOADTABFILE
#define IPROP_SPREAD_LOADTABFILE         160
#endif
#else
#ifndef SSP_NO_COMBOHWND
#define IPROP_SPREAD_COMBOHWND           160
#endif
#endif // SS_V30
#ifndef SSP_NO_MAXTEXTROWHEIGHT
#define IPROP_SPREAD_MAXTEXTROWHEIGHT    161
#endif
#ifndef SSP_NO_MAXTEXTCELLHEIGHT
#define IPROP_SPREAD_MAXTEXTCELLHEIGHT   162
#endif
#ifndef SSP_NO_MAXTEXTCOLWIDTH
#define IPROP_SPREAD_MAXTEXTCOLWIDTH     163
#endif
#ifndef SSP_NO_MAXTEXTCELLWIDTH
#define IPROP_SPREAD_MAXTEXTCELLWIDTH    164
#endif
#ifndef SSP_NO_DATACOLCNT
#define IPROP_SPREAD_DATACOLCNT          165
#endif
#ifndef SSP_NO_DATAROWCNT
#define IPROP_SPREAD_DATAROWCNT          166
#endif
#ifndef SSP_NO_AUTOCLIPBOARD
#define IPROP_SPREAD_AUTOCLIPBOARD       167
#endif
#ifndef SSP_NO_ALLOWMULTIBLOCKS
#define IPROP_SPREAD_ALLOWMULTIBLOCKS    168
#endif
#ifndef SSP_NO_ALLOWCELLOVERFLOW
#define IPROP_SPREAD_ALLOWCELLOVERFLOW   169
#endif
#ifndef SSP_NO_ALLOWDRAGDROP
#define IPROP_SPREAD_ALLOWDRAGDROP       170
#endif
#ifndef SSP_NO_GRIDLINECOLOR
#define IPROP_SPREAD_GRIDLINECOLOR       171
#endif
#ifndef SSP_NO_GRIDSOLID
#define IPROP_SPREAD_GRIDSOLID           172
#endif
#ifndef SSP_NO_VSCROLLSPECIAL
#define IPROP_SPREAD_VSCROLLSPECIAL      173
#endif
#ifndef SSP_NO_USERRESIZE
#define IPROP_SPREAD_USERRESIZE          174
#endif
#ifndef SSP_NO_BUTTONDRAWMODE
#define IPROP_SPREAD_BUTTONDRAWMODE      175
#endif
#ifndef SSP_NO_SELECTBLOCKOPTIONS
#define IPROP_SPREAD_SELECTBLOCKOPTIONS  176
#endif
#ifndef SSP_NO_TYPEFLOATDEFCUR
#define IPROP_SPREAD_TYPEFLOATDEFCUR     177
#endif
#ifndef SSP_NO_TYPEFLOATDEFDECIMAL
#define IPROP_SPREAD_TYPEFLOATDEFDECIMAL 178
#endif
#ifndef SSP_NO_TYPEFLOATDEFSEP
#define IPROP_SPREAD_TYPEFLOATDEFSEP     179
#endif
#ifndef SSP_NO_VIRTUALROWS
#define IPROP_SPREAD_VIRTUALROWS         180
#endif
#ifndef SSP_NO_VIRTUALOVERLAP
#define IPROP_SPREAD_VIRTUALOVERLAP      181
#endif
#ifndef SSP_NO_VIRTUALMODE
#define IPROP_SPREAD_VIRTUALMODE         182
#endif
#ifndef SSP_NO_VIRTUALMAXROWS
#define IPROP_SPREAD_VIRTUALMAXROWS      183
#endif
#ifndef SSP_NO_VIRTUALSCROLLBUFFER
#define IPROP_SPREAD_VIRTUALSCROLLBUFFER 184
#endif
#ifndef SSP_NO_NOBORDER
#define IPROP_SPREAD_NOBORDER            185
#endif
#ifndef SSP_NO_GRIDSHOWHORIZ
#define IPROP_SPREAD_GRIDSHOWHORIZ       186
#endif
#ifndef SSP_NO_GRIDSHOWVERT
#define IPROP_SPREAD_GRIDSHOWVERT        187
#endif
#ifndef SSP_NO_STARTINGCOLNUMBER
#define IPROP_SPREAD_STARTINGCOLNUMBER   188
#endif
#ifndef SSP_NO_STARTINGROWNUMBER
#define IPROP_SPREAD_STARTINGROWNUMBER   189
#endif
#ifndef SSP_NO_COLHEADERDISPLAY
#define IPROP_SPREAD_COLHEADERDISPLAY    190
#endif
#ifndef SSP_NO_ROWHEADERDISPLAY
#define IPROP_SPREAD_ROWHEADERDISPLAY    191
#endif
#ifndef SSP_NO_SORTKEYORDER
#define IPROP_SPREAD_SORTKEYORDER        192
#endif
#ifndef SSP_NO_SORTKEY
#define IPROP_SPREAD_SORTKEY             193
#endif
#ifndef SSP_NO_SORTBY
#define IPROP_SPREAD_SORTBY              194
#endif
#ifndef SSP_NO_CHANGEMADE
#define IPROP_SPREAD_CHANGEMADE          195
#endif
#ifndef SSP_NO_VSCROLLSPECIALTYPE
#define IPROP_SPREAD_VSCROLLSPECIALTYPE  196
#endif
#ifndef SSP_NO_USERRESIZEROW
#define IPROP_SPREAD_USERRESIZEROW       197
#endif
#ifndef SSP_NO_USERRESIZECOL
#define IPROP_SPREAD_USERRESIZECOL       198
#endif
#ifndef SSP_NO_ROWHIDDEN
#define IPROP_SPREAD_ROWHIDDEN           199
#endif
#ifndef SSP_NO_COLHIDDEN
#define IPROP_SPREAD_COLHIDDEN           200
#endif
#ifndef SSP_NO_TYPEFLOATCURRENCY
#define IPROP_SPREAD_TYPEFLOATCURRENCY   201
#endif
#ifndef SSP_NO_TYPEFLOATDECIMAL
#define IPROP_SPREAD_TYPEFLOATDECIMAL    202
#endif
#ifndef SSP_NO_TYPEFLOATSEPCHAR
#define IPROP_SPREAD_TYPEFLOATSEPCHAR    203
#endif
#ifndef SSP_NO_CURSORTYPE
#define IPROP_SPREAD_CURSORTYPE          204
#endif
#ifndef SSP_NO_CURSORSTYLE
#define IPROP_SPREAD_CURSORSTYLE         205
#endif
#ifndef SSP_NO_CURSORICON
#define IPROP_SPREAD_CURSORICON          206
#endif
#ifndef SSP_NO_BORDERTYPE
#define IPROP_SPREAD_BORDERTYPE          207
#endif
#ifndef SSP_NO_BORDERSTYLE
#define IPROP_SPREAD_BORDERSTYLE         208
#endif
#ifndef SSP_NO_BORDERCOLOR
#define IPROP_SPREAD_BORDERCOLOR         209
#endif
#ifndef SSP_NO_MULTISELINDEX
#define IPROP_SPREAD_MULTISELINDEX       210
#endif
#ifndef SSP_NO_MULTISELCOUNT
#define IPROP_SPREAD_MULTISELCOUNT       211
#endif
#ifndef SSP_NO_DESTCOL
#define IPROP_SPREAD_DESTCOL             212
#endif
#ifndef SSP_NO_DESTROW
#define IPROP_SPREAD_DESTROW             213
#endif
#ifndef SSP_NO_COLPAGEBREAK
#define IPROP_SPREAD_COLPAGEBREAK        214
#endif
#ifndef SSP_NO_ROWPAGEBREAK
#define IPROP_SPREAD_ROWPAGEBREAK        215
#endif
#ifndef SSP_NO_VIRTUALCURROWCOUNT
#define IPROP_SPREAD_VIRTUALCURROWCOUNT  216
#endif
#ifndef SSP_NO_VIRTUALCURTOP
#define IPROP_SPREAD_VIRTUALCURTOP       217
#endif
#ifndef SSP_NO_TYPESPIN
#define IPROP_SPREAD_TYPESPIN            218
#endif
#ifndef SSP_NO_TYPEINTSPINWRAP
#define IPROP_SPREAD_TYPEINTSPINWRAP     219
#endif
#ifndef SSP_NO_TYPEINTSPININC
#define IPROP_SPREAD_TYPEINTSPININC      220
#endif
#ifndef SSP_NO_TYPETEXTPREFIX
#define IPROP_SPREAD_TYPETEXTPREFIX      221
#endif
#ifndef SSP_NO_TYPETEXTSHADOWIN
#define IPROP_SPREAD_TYPETEXTSHADOWIN    222
#endif
#ifndef SSP_NO_TYPECOMBOEDITABLE
#define IPROP_SPREAD_TYPECOMBOEDITABLE   223
#endif
#ifndef SSP_NO_OWNERDRAWSTYLE
#define IPROP_SPREAD_OWNERDRAWSTYLE      224
#endif
#ifndef SSP_NO_CHECKCENTER
#define IPROP_SPREAD_CHECKCENTER         225
#endif
#ifndef SSP_NO_CHECKTEXT
#define IPROP_SPREAD_CHECKTEXT           226
#endif
#ifndef SSP_NO_CHECKPICTURE
#define IPROP_SPREAD_CHECKPICTURE        227
#endif
#ifndef SSP_NO_ABOUT
#define IPROP_SPREAD_ABOUT               228
#endif
#ifndef SSP_NO_CHECKTEXTALIGN
#define IPROP_SPREAD_CHECKTEXTALIGN      229
#endif
#ifndef SSP_NO_COMBOINDEX
#define IPROP_SPREAD_COMBOINDEX          230
#endif
#ifndef SSP_NO_COMBOSTRING
#define IPROP_SPREAD_COMBOSTRING         231
#endif
#ifndef SSP_NO_COMBOCOUNT
#define IPROP_SPREAD_COMBOCOUNT          232
#endif
#ifndef SSP_NO_TYPEHALIGN
#define IPROP_SPREAD_TYPEHALIGN          233
#endif
#ifndef SSP_NO_PRINTHDCPRINTER
#define IPROP_SPREAD_PRINTHDCPRINTER     234
#endif
#ifndef SSP_NO_DATASOURCE
#define IPROP_SPREAD_DATASOURCE          235
#endif
#ifndef SSP_NO_DATACHANGED
#define IPROP_SPREAD_DATACHANGED         236
#endif
#ifndef SSP_NO_DAUTOHEADINGS
#define IPROP_SPREAD_DAUTOHEADINGS       237
#endif
#ifndef SSP_NO_DAUTOCELLTYPES
#define IPROP_SPREAD_DAUTOCELLTYPES      238
#endif
#ifndef SSP_NO_DAUTOFILL
#define IPROP_SPREAD_DAUTOFILL           239
#endif
#ifndef SSP_NO_DAUTOSIZECOLS
#define IPROP_SPREAD_DAUTOSIZECOLS       240
#endif
#ifndef SSP_NO_DATAFIELD
#define IPROP_SPREAD_DATAFIELD           241
#endif
#ifndef SSP_NO_DATAFILLEVENT
#define IPROP_SPREAD_DATAFILLEVENT       242
#endif
#ifndef SSP_NO_DINFORMACTIVEROW
#define IPROP_SPREAD_DINFORMACTIVEROW    243
#endif
#ifndef SSP_NO_COMBOCURSEL
#define IPROP_SPREAD_COMBOCURSEL         244
#endif
#ifndef SSP_NO_SELMODEINDEX
#define IPROP_SPREAD_SELMODEINDEX        245
#endif
#ifndef SSP_NO_SELMODESELCOUNT
#define IPROP_SPREAD_SELMODESELCOUNT     246
#endif
#ifndef SSP_NO_SELMODESELECTED
#define IPROP_SPREAD_SELMODESELECTED     247
#endif
#ifndef SSP_NO_HELPCONTEXTID
#define IPROP_SPREAD_HELPCONTEXTID       248
#endif
#ifndef SSP_NO_DAUTOSAVE
#define IPROP_SPREAD_DAUTOSAVE           249
#endif
#ifndef SS_V30
#ifndef SSP_NO_DATASELECT
#ifdef SS_QE
#define IPROP_SPREAD_DATASELECT          250
#endif
#endif
#else
#ifndef SSP_NO_SELBACKCOLOR
#define IPROP_SPREAD_SELBACKCOLOR        250
#endif
#endif // !SS_V30
#ifndef SSP_NO_BACKCOLORSTYLE
#define IPROP_SPREAD_BACKCOLORSTYLE      251
#endif
#ifndef SSP_NO_CHECKTYPE
#define IPROP_SPREAD_CHECKTYPE           252
#endif
#ifndef SS_V30
#define IPROP_SPREAD_CNT                 253
#else // SS_V30
#ifndef SSP_NO_SELFORECOLOR
#define IPROP_SPREAD_SELFORECOLOR        253
#endif
#define IPROP_SPREAD_CNT                 254
#endif // SS_V30

#ifdef SS_GP
#define IPROP_SPREAD_EDITEXPWIDTH        122
#define IPROP_SPREAD_EDITEXPROWCNT       123
#endif

#define IPROP_SPREAD_CHECKPICTUP         500
#define IPROP_SPREAD_CHECKPICTDOWN       501
#define IPROP_SPREAD_CHECKPICTFOCUSUP    502
#define IPROP_SPREAD_CHECKPICTFOCUSDOWN  503
#define IPROP_SPREAD_CHECKPICTGRAY       504
#define IPROP_SPREAD_CHECKPICTFOCUSGRAY  505

enum 
   {
   IEVENT_SPREAD_ADVANCE,
   IEVENT_SPREAD_BLOCKSELECTED,
   IEVENT_SPREAD_BUTTONCLICKED,
   IEVENT_SPREAD_CHANGE,
   IEVENT_SPREAD_CLICK,
   IEVENT_SPREAD_DBLCLICK,
   IEVENT_SPREAD_DRAGDROP,
   IEVENT_SPREAD_DRAGOVER,
   IEVENT_SPREAD_EDITERROR,
   IEVENT_SPREAD_EDITMODE,
   IEVENT_SPREAD_GOTFOCUS,
   IEVENT_SPREAD_KEYDOWN,
   IEVENT_SPREAD_KEYPRESS,
   IEVENT_SPREAD_KEYUP,
   IEVENT_SPREAD_LEAVECELL,
   IEVENT_SPREAD_LOSTFOCUS,
   IEVENT_SPREAD_MOUSEDOWN,
   IEVENT_SPREAD_MOUSEMOVE,
   IEVENT_SPREAD_MOUSEUP,
   IEVENT_SPREAD_SELCHANGE,
   IEVENT_SPREAD_QUERYADVANCE,
   IEVENT_SPREAD_QUERYDATA,
   IEVENT_SPREAD_COLWIDTHCHANGE,
   IEVENT_SPREAD_ROWHEIGHTCHANGE,
   IEVENT_SPREAD_ENTERROW,
   IEVENT_SPREAD_USERFORMULAENTERED,
   IEVENT_SPREAD_RIGHTCLICK,
   IEVENT_SPREAD_LEAVEROW,
   IEVENT_SPREAD_DRAGDROPBLOCK,
   IEVENT_SPREAD_TOPLEFTCHANGE,
   IEVENT_SPREAD_DRAWITEM,
   IEVENT_SPREAD_DATACOLCONFIG,
   IEVENT_SPREAD_DATAFILL,
   IEVENT_SPREAD_PRINTABORT,
   IEVENT_SPREAD_CUSTOMFUNCTION,
   IEVENT_SPREAD_VIRTUALCLEARDATA,
   IEVENT_SPREAD_DATAADDNEW,
   IEVENT_SPREAD_INSDEL,
   IEVENT_SPREAD_PRINTMSGBOX,
   IEVENT_SPREAD_ARROWPRESSED,
#ifdef SS_V30
   IEVENT_SPREAD_COMBOCLOSEUP,
   IEVENT_SPREAD_COMBODROPDOWN,
   IEVENT_SPREAD_COMBOSELCHANGE,
   IEVENT_SPREAD_TEXTTIPFETCH,
   IEVENT_SPREAD_EDITCHANGE,
#endif
#ifdef SS_V35
   IEVENT_SPREAD_BEFOREUSERSORT,
   IEVENT_SPREAD_AFTERUSERSORT,
#endif
   IEVENT_SPREAD_CNT
   };

enum 
   {
#ifndef SSE_NO_ADVANCE
   IEVENTVB1_SPREAD_ADVANCE,
#endif
#ifndef SSE_NO_ARROWPRESSED
   IEVENTVB1_SPREAD_ARROWPRESSED,
#endif
#ifndef SSE_NO_BLOCKSELECTED
   IEVENTVB1_SPREAD_BLOCKSELECTED,
#endif
#ifndef SSE_NO_BUTTONCLICKED
   IEVENTVB1_SPREAD_BUTTONCLICKED,
#endif
#ifndef SSE_NO_CHANGE
   IEVENTVB1_SPREAD_CHANGE,
#endif
#ifndef SSE_NO_CLICK
   IEVENTVB1_SPREAD_CLICK,
#endif
#ifndef SSE_NO_COLWIDTHCHANGE
   IEVENTVB1_SPREAD_COLWIDTHCHANGE,
#endif
#ifdef SS_V30
#ifndef SSE_NO_COMBOCLOSEUP
   IEVENTVB1_SPREAD_COMBOCLOSEUP,
#endif
#ifndef SSE_NO_COMBODROPDOWN
   IEVENTVB1_SPREAD_COMBODROPDOWN,
#endif
#ifndef SSE_NO_COMBOSELCHANGE
   IEVENTVB1_SPREAD_COMBOSELCHANGE,
#endif
#endif
#ifndef SSE_NO_CUSTOMFUNCTION
   IEVENTVB1_SPREAD_CUSTOMFUNCTION,
#endif
#ifndef SSE_NO_DATAADDNEW
   IEVENTVB1_SPREAD_DATAADDNEW,
#endif
#ifndef SSE_NO_DATACOLCONFIG
   IEVENTVB1_SPREAD_DATACOLCONFIG,
#endif
#ifndef SSE_NO_DATAFILL
   IEVENTVB1_SPREAD_DATAFILL,
#endif
#ifndef SSE_NO_DBLCLICK
   IEVENTVB1_SPREAD_DBLCLICK,
#endif
#ifndef SSE_NO_DRAGDROP
   IEVENTVB1_SPREAD_DRAGDROP,
#endif
#ifndef SSE_NO_DRAGDROPBLOCK
   IEVENTVB1_SPREAD_DRAGDROPBLOCK,
#endif
#ifndef SSE_NO_DRAGOVER
   IEVENTVB1_SPREAD_DRAGOVER,
#endif
#ifndef SSE_NO_DRAWITEM
   IEVENTVB1_SPREAD_DRAWITEM,
#endif
#ifdef SS_V30
#ifndef SSE_NO_EDITCHANGE
   IEVENTVB1_SPREAD_EDITCHANGE,
#endif
#endif
#ifndef SSE_NO_EDITERROR
   IEVENTVB1_SPREAD_EDITERROR,
#endif
#ifndef SSE_NO_EDITMODE
   IEVENTVB1_SPREAD_EDITMODE,
#endif
#ifndef SSE_NO_ENTERROW
   IEVENTVB1_SPREAD_ENTERROW,
#endif
#ifndef SSE_NO_GOTFOCUS
   IEVENTVB1_SPREAD_GOTFOCUS,
#endif
#ifndef SSE_NO_INSDEL
   IEVENTVB1_SPREAD_INSDEL,
#endif
#ifndef SSE_NO_KEYDOWN
   IEVENTVB1_SPREAD_KEYDOWN,
#endif
#ifndef SSE_NO_KEYPRESS
   IEVENTVB1_SPREAD_KEYPRESS,
#endif
#ifndef SSE_NO_KEYUP
   IEVENTVB1_SPREAD_KEYUP,
#endif
#ifndef SSE_NO_LEAVECELL
   IEVENTVB1_SPREAD_LEAVECELL,
#endif
#ifndef SSE_NO_LEAVEROW
   IEVENTVB1_SPREAD_LEAVEROW,
#endif
#ifndef SSE_NO_LOSTFOCUS
   IEVENTVB1_SPREAD_LOSTFOCUS,
#endif
#ifndef SSE_NO_MOUSEDOWN
   IEVENTVB1_SPREAD_MOUSEDOWN,
#endif
#ifndef SSE_NO_MOUSEMOVE
   IEVENTVB1_SPREAD_MOUSEMOVE,
#endif
#ifndef SSE_NO_MOUSEUP
   IEVENTVB1_SPREAD_MOUSEUP,
#endif
#ifndef SSE_NO_PRINTABORT
   IEVENTVB1_SPREAD_PRINTABORT,
#endif
#ifndef SSE_NO_PRINTMSGBOX
   IEVENTVB1_SPREAD_PRINTMSGBOX,
#endif
#ifndef SSE_NO_QUERYADVANCE
   IEVENTVB1_SPREAD_QUERYADVANCE,
#endif
#ifndef SSE_NO_QUERYDATA
   IEVENTVB1_SPREAD_QUERYDATA,
#endif
#ifndef SSE_NO_RIGHTCLICK
   IEVENTVB1_SPREAD_RIGHTCLICK,
#endif
#ifndef SSE_NO_ROWHEIGHTCHANGE
   IEVENTVB1_SPREAD_ROWHEIGHTCHANGE,
#endif
#ifndef SSE_NO_SELCHANGE
   IEVENTVB1_SPREAD_SELCHANGE,
#endif
#ifdef SS_V30
#ifndef SSE_NO_TEXTTIPFETCH
   IEVENTVB1_SPREAD_TEXTTIPFETCH,
#endif
#endif
#ifndef SSE_NO_TOPLEFTCHANGE
   IEVENTVB1_SPREAD_TOPLEFTCHANGE,
#endif
#ifndef SSE_NO_USERFORMULAENTERED
   IEVENTVB1_SPREAD_USERFORMULAENTERED,
#endif
#ifndef SSE_NO_VIRTUALCLEARDATA
   IEVENTVB1_SPREAD_VIRTUALCLEARDATA,
#endif
   IEVENTVB1_SPREAD_CNT
   };

#define IDBMP_SPREAD                   8030
#define IDBMP_SPREADDOWN               8031
#define IDBMP_SPREADMONO               8033
#define IDBMP_SPREADEGA                8036

#define CELLTYPE_DATE                  0
#define CELLTYPE_EDIT                  1
#define CELLTYPE_FLOAT                 2
#define CELLTYPE_INTEGER               3
#define CELLTYPE_PIC                   4
#define CELLTYPE_STATICTEXT            5
#define CELLTYPE_TIME                  6
#define CELLTYPE_BUTTON                7
#define CELLTYPE_COMBOBOX              8
#define CELLTYPE_PICTURE               9
#define CELLTYPE_CHECKBOX              10
#define CELLTYPE_OWNERDRAW             11

typedef struct vbssprint
   {
   GLOBALHANDLE  hPrintJobName;
   GLOBALHANDLE  hPrintAbortMsg;
   GLOBALHANDLE  hPrintHeader;
   GLOBALHANDLE  hPrintFooter;
   short         PrintBorder;
   short         PrintShadows;
   short         PrintGrid;
   short         PrintColHeaders;
   short         PrintRowHeaders;
   short         PrintUseDataMax;
   short         PrintType;
   short         PrintPageStart;
   short         PrintPageEnd;
   short         PrintOrientation;
   long          PrintMarginLeft;
   long          PrintMarginTop;
   long          PrintMarginRight;
   long          PrintMarginBottom;
   short         PrintColor;
   short         PrinthDCPrinter;
   HWND          hWndPrintDlgProc;
#ifdef SS_V30
   BOOL          fSmartPrint;
   short         nPageOrder;
   long          lFirstPageNumber;
#endif
   } VBSSPRINT, FAR *LPVBSSPRINT;

typedef struct vbspread
   {
#ifdef SS_VB
   long          SelStart;
   short         BlockMode;
   COLORREF      ShadowColor;
   COLORREF      ShadowText;
   COLORREF      ShadowDark;
   short         CellType;
   short         FileNum;
   short         GridLines;
   short         Position;
   short         nMaxEditLen;
   short         TypeEditCharSet;
   short         TypeEditCharCase;
   short         TypeEditPassword;
   short         TypeEditMultiLine;
   double        TypeEditWidth;
   short         TypeEditRowCnt;
   short         TypeTextJustifyVert;
   short         TypeTextWordWrap;
   short         TypeTextShadow;
   DATEFORMAT    TypeDateFormat;
   TIMEFORMAT    TypeTimeFormat;
   long          TypeIntegerMin;
   long          TypeIntegerMax;
   short         TypeFloatSeparator;
   short         TypeFloatMoney;
   short         TypeFloatDecimalPlaces;
   short         TypeButtonAlign;
   short         TypeButtonType;
   SUPERBTNCOLOR TypeButtonColor;
   short         TypeButtonShadowSize;
   short         dSortBy;
   SS_COORD      ColDest;
   SS_COORD      RowDest;
   WORD          wBorderType;
   WORD          wBorderStyle;
   COLORREF      BorderColor;
   short         dCursorType;
   VBSSPRINT     Print;
#endif // SS_VB

#ifdef SS_BOUNDCONTROL
   DATAOPTS      DataOpts;
   SS_COORD      RowBeingInserted;
   SS_COORD      RowBeingSaved;
   short         dFieldsCount;
   BOOL          fUpdatable;
   BOOL          fBookmarkable;
   HLSTR         hlstrDataFillBookMark;
   BOOL          fInternalSetBookmark;
   BOOL          fReReadRow;
   HLSTR         hlstrLastBookMark;    // Used to determine if ADD worked
   BOOL          fIgnoreSaveBoundRow;
   BOOL          fIgnoreMoveNext;
   BOOL          fAddNewFromDataControl;
   BOOL          fDataDeleteInternal;
   BOOL          fUpdateReceived;
   BOOL          fVRefreshCurrent;
   short         dDataGetFillCol;
   short         dDataSetFillCol;
   SS_COORD      PostAddRowNew;
   SS_COORD      PostAddRowNewCol;
   BOOL          fUReadNext;
   BOOL          fDataIgnoreDataRequest;
#endif   

   SS_COORD      Col;
   SS_COORD      Row;
   BOOL          fBoundActive;
   SS_COORD      fChangeMsgPosted;

   // These are not set or retrieved by VB.  In other words, they
   // are not used by properties that have the PF_fSetData or
   // PF_fGetData flags set

#ifdef SS_VB
   GLOBALHANDLE  hSortKeys;
   short         nSortKeyCnt;
   SS_COORD      Col2;
   SS_COORD      Row2;
   BOOL          CalcDependencies;
   GLOBALHANDLE  hTypePicMask;
   GLOBALHANDLE  hTypePicDefText;
   GLOBALHANDLE  hTypeComboList;
   TB_DATE       TypeDateMin;
   TB_DATE       TypeDateMax;
   TIME          TypeTimeMin;
   TIME          TypeTimeMax;
   double        TypeFloatMin;
   double        TypeFloatMax;
   FLOATFORMAT   TypeFloatFormat;
   FLOATFORMAT   TypeFloatDefFormat;
   GLOBALHANDLE  hTypeButtonText;
   HANDLE        hTypeButtonPictureName;
   HANDLE        hTypeButtonPictureDownName;
   short         TypeButtonPictureType;
   short         TypeButtonPictureDownType;
   SS_COORD      lMaxCols;
   SS_COORD      lMaxRows;
   HANDLE        hTypePictPicture;
   HANDLE        hTypePictPal;
   short         TypePictStyle;
   BOOL          fWantSpecialKeyDown;
   HRGN          hFontRgn;
   BOOL          fFontRgnSet;
   GLOBALHANDLE  hMultiSelBlocks;
   short         dMultiSelBlockCnt;
   short         dMultiSelBlockIndex;
   BOOL          bSpin;
   BOOL          fIntegerSpinWrap;
   long          lIntegerSpinInc;
   BOOL          fTextPrefix;
   BOOL          fTextShadowIn;
   BOOL          fComboEditable;
   short         dComboMaxDrop;
   short         dComboWidth;
   short         dComboMaxEditLen;
   long          lOwnerDrawStyle;
   BOOL          fCheckCenter;
   short         nCheckType;
   BYTE          bCheckTextAlign;
   GLOBALHANDLE  hCheckText;
   HANDLE        hCheckPictUp;
   short         dCheckPictUpType;
   HANDLE        hCheckPictDown;
   short         dCheckPictDownType;
   HANDLE        hCheckPictFocusUp;
   short         dCheckPictFocusUpType;
   HANDLE        hCheckPictFocusDown;
   short         dCheckPictFocusDownType;
   HANDLE        hCheckPictGray;
   short         dCheckPictGrayType;
   HANDLE        hCheckPictFocusGray;
   short         dCheckPictFocusGrayType;
   HICON         hCursorIcon;
   short         dErrorMode;
   HPIC          hPic;
   BYTE          fSaveDesignerInfo;
   HWND          hWndDesigner;
   short         dTypeComboIndex;
   LPSS_VALUE    lpCFValues;
   short         dCFValueCnt;
   LPSS_VALUE    lpCFResult;
   HWND          hWndPropPopup;
   BOOL          fUtopiaMode;
   BOOL          fVB1;
   BOOL          fVBMLoaded;
   TBGLOBALHANDLE hClassExtraBytes;
#ifdef SS_QE
   DBINFO        DBInfo;
#endif
#endif // SS_VB
   } VBSPREAD, FAR *LPVBSPREAD;

typedef struct spreadparams1
   {
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS1;

typedef struct spreadparams2
   {
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS2;

typedef struct spreadparams3
   {
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS3;

typedef struct spreadparams4
   {
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS4;

typedef struct spreadparams5
   {
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS5;

typedef struct spreadparams6
   {
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS6;

typedef struct spreadparams7
   {
   LPVOID lpParam7;
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS7;

typedef struct spreadparams8
   {
   LPVOID lpParam8;
   LPVOID lpParam7;
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS8;

typedef struct spreadparams12
   {
   LPVOID lpParam12;
   LPVOID lpParam11;
   LPVOID lpParam10;
   LPVOID lpParam9;
   LPVOID lpParam8;
   LPVOID lpParam7;
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } SPREADPARAMS12;

#define ALLROWS -1
#define ALLCOLS -1

#define VBSS_FILESAVESIZE 16384
#define VBSS_FILEREADSIZE 2048

#define VBM_ISMNEMONIC     ( VBM__BASE + 0x26 )
#define VBM_WANTSPECIALKEY ( VBM__BASE + 0x27 )
#define SS_ISVB1           (lpSpread->fVB1)

#define IDD_OPENDB_SOURCE   101
#define IDD_OPENDB_DB       102
#define IDD_OPENDB_USERID   103
#define IDD_OPENDB_PASSWORD 104

#ifndef SSE_NO_ADVANCE
#define SS_EVENT_Advance       (SS_ISVB1 ? IEVENTVB1_SPREAD_ADVANCE : \
                                IEVENT_SPREAD_ADVANCE)
#endif
#ifndef SSE_NO_BLOCKSELECTED
#define SS_EVENT_BlockSelected (SS_ISVB1 ? IEVENTVB1_SPREAD_BLOCKSELECTED : \
                                IEVENT_SPREAD_BLOCKSELECTED)
#endif
#ifndef SSE_NO_BUTTONCLICKED
#define SS_EVENT_ButtonClicked (SS_ISVB1 ? IEVENTVB1_SPREAD_BUTTONCLICKED : \
                                IEVENT_SPREAD_BUTTONCLICKED)
#endif
#ifndef SSE_NO_CHANGE
#define SS_EVENT_Change        (SS_ISVB1 ? IEVENTVB1_SPREAD_CHANGE : \
                                IEVENT_SPREAD_CHANGE)
#endif
#ifndef SSE_NO_CLICK
#define SS_EVENT_Click         (SS_ISVB1 ? IEVENTVB1_SPREAD_CLICK : \
                                IEVENT_SPREAD_CLICK)
#endif
#ifndef SSE_NO_DBLCLICK
#define SS_EVENT_DblClick      (SS_ISVB1 ? IEVENTVB1_SPREAD_DBLCLICK : \
                                IEVENT_SPREAD_DBLCLICK)
#endif
#ifndef SSE_NO_DRAGDROP
#define SS_EVENT_DragDrop      (SS_ISVB1 ? IEVENTVB1_SPREAD_DRAGDROP : \
                                IEVENT_SPREAD_DRAGDROP)
#endif
#ifndef SSE_NO_DRAGOVER
#define SS_EVENT_DragOver      (SS_ISVB1 ? IEVENTVB1_SPREAD_DRAGOVER : \
                                IEVENT_SPREAD_DRAGOVER)
#endif
#ifndef SSE_NO_EDITERROR
#define SS_EVENT_EditError     (SS_ISVB1 ? IEVENTVB1_SPREAD_EDITERROR : \
                                IEVENT_SPREAD_EDITERROR)
#endif
#ifndef SSE_NO_EDITMODE
#define SS_EVENT_EditMode      (SS_ISVB1 ? IEVENTVB1_SPREAD_EDITMODE : \
                                IEVENT_SPREAD_EDITMODE)
#endif
#ifndef SSE_NO_GOTFOCUS
#define SS_EVENT_GotFocus      (SS_ISVB1 ? IEVENTVB1_SPREAD_GOTFOCUS : \
                                IEVENT_SPREAD_GOTFOCUS)
#endif
#ifndef SSE_NO_KEYDOWN
#define SS_EVENT_KeyDown       (SS_ISVB1 ? IEVENTVB1_SPREAD_KEYDOWN : \
                                IEVENT_SPREAD_KEYDOWN)
#endif
#ifndef SSE_NO_KEYPRESS
#define SS_EVENT_KeyPress      (SS_ISVB1 ? IEVENTVB1_SPREAD_KEYPRESS : \
                                IEVENT_SPREAD_KEYPRESS)
#endif
#ifndef SSE_NO_KEYUP
#define SS_EVENT_KeyUp         (SS_ISVB1 ? IEVENTVB1_SPREAD_KEYUP : \
                                IEVENT_SPREAD_KEYUP)
#endif
#ifndef SSE_NO_LEAVECELL
#define SS_EVENT_LeaveCell     (SS_ISVB1 ? IEVENTVB1_SPREAD_LEAVECELL : \
                                IEVENT_SPREAD_LEAVECELL)
#endif
#ifndef SSE_NO_LOSTFOCUS
#define SS_EVENT_LostFocus     (SS_ISVB1 ? IEVENTVB1_SPREAD_LOSTFOCUS : \
                                IEVENT_SPREAD_LOSTFOCUS)
#endif
#ifndef SSE_NO_MOUSEDOWN
#define SS_EVENT_MouseDown     (SS_ISVB1 ? IEVENTVB1_SPREAD_MOUSEDOWN : \
                                IEVENT_SPREAD_MOUSEDOWN)
#endif
#ifndef SSE_NO_MOUSEMOVE
#define SS_EVENT_MouseMove     (SS_ISVB1 ? IEVENTVB1_SPREAD_MOUSEMOVE : \
                                IEVENT_SPREAD_MOUSEMOVE)
#endif
#ifndef SSE_NO_MOUSEUP
#define SS_EVENT_MouseUp       (SS_ISVB1 ? IEVENTVB1_SPREAD_MOUSEUP : \
                                IEVENT_SPREAD_MOUSEUP)
#endif
#ifndef SSE_NO_SELCHANGE
#define SS_EVENT_SelChange     (SS_ISVB1 ? IEVENTVB1_SPREAD_SELCHANGE : \
                                IEVENT_SPREAD_SELCHANGE)
#endif
#ifndef SSE_NO_QUERYADVANCE
#define SS_EVENT_QueryAdvance  (SS_ISVB1 ? IEVENTVB1_SPREAD_QUERYADVANCE : \
                                IEVENT_SPREAD_QUERYADVANCE)
#endif
#ifndef SSE_NO_QUERYDATA
#define SS_EVENT_QueryData     (SS_ISVB1 ? IEVENTVB1_SPREAD_QUERYDATA : \
                                IEVENT_SPREAD_QUERYDATA)
#endif
#ifndef SSE_NO_COLWIDTHCHANGE
#define SS_EVENT_ColWidthChange (SS_ISVB1 ? IEVENTVB1_SPREAD_COLWIDTHCHANGE : \
                                IEVENT_SPREAD_COLWIDTHCHANGE)
#endif
#ifndef SSE_NO_ROWHEIGHTCHANGE
#define SS_EVENT_RowHeightChange (SS_ISVB1 ? IEVENTVB1_SPREAD_ROWHEIGHTCHANGE : \
                                IEVENT_SPREAD_ROWHEIGHTCHANGE)
#endif
#ifndef SSE_NO_ENTERROW
#define SS_EVENT_EnterRow      (SS_ISVB1 ? IEVENTVB1_SPREAD_ENTERROW : \
                                IEVENT_SPREAD_ENTERROW)
#endif
#ifndef SSE_NO_USERFORMULAENTERED
#define SS_EVENT_UserFormulaEntered (SS_ISVB1 ? IEVENTVB1_SPREAD_USERFORMULAENTERED : \
                                IEVENT_SPREAD_USERFORMULAENTERED)
#endif
#ifndef SSE_NO_RIGHTCLICK
#define SS_EVENT_RightClick    (SS_ISVB1 ? IEVENTVB1_SPREAD_RIGHTCLICK : \
                                IEVENT_SPREAD_RIGHTCLICK)
#endif
#ifndef SSE_NO_LEAVEROW
#define SS_EVENT_LeaveRow      (SS_ISVB1 ? IEVENTVB1_SPREAD_LEAVEROW : \
                                IEVENT_SPREAD_LEAVEROW)
#endif
#ifndef SSE_NO_DRAGDROPBLOCK
#define SS_EVENT_DragDropBlock (SS_ISVB1 ? IEVENTVB1_SPREAD_DRAGDROPBLOCK : \
                                IEVENT_SPREAD_DRAGDROPBLOCK)
#endif
#ifndef SSE_NO_TOPLEFTCHANGE
#define SS_EVENT_TopLeftChange (SS_ISVB1 ? IEVENTVB1_SPREAD_TOPLEFTCHANGE : \
                                IEVENT_SPREAD_TOPLEFTCHANGE)
#endif
#ifndef SSE_NO_DRAWITEM
#define SS_EVENT_DrawItem      (SS_ISVB1 ? IEVENTVB1_SPREAD_DRAWITEM : \
                                IEVENT_SPREAD_DRAWITEM)
#endif
#ifndef SSE_NO_DATACOLCONFIG
#define SS_EVENT_DataColConfig (SS_ISVB1 ? IEVENTVB1_SPREAD_DATACOLCONFIG : \
                                IEVENT_SPREAD_DATACOLCONFIG)
#endif
#ifndef SSE_NO_DATAFILL
#define SS_EVENT_DataFill      (SS_ISVB1 ? IEVENTVB1_SPREAD_DATAFILL : \
                                IEVENT_SPREAD_DATAFILL)
#endif
#ifndef SSE_NO_PRINTABORT
#define SS_EVENT_PrintAbort    (SS_ISVB1 ? IEVENTVB1_SPREAD_PRINTABORT : \
                                IEVENT_SPREAD_PRINTABORT)
#endif
#define SS_EVENT_PrintMsgBox   (SS_ISVB1 ? IEVENTVB1_SPREAD_PRINTMSGBOX : \
                                IEVENT_SPREAD_PRINTMSGBOX)
#ifndef SSE_NO_CUSTOMFUNCTION
#define SS_EVENT_CustomFunction (SS_ISVB1 ? IEVENTVB1_SPREAD_CUSTOMFUNCTION : \
                                IEVENT_SPREAD_CUSTOMFUNCTION)
#endif
#ifndef SSE_NO_VIRTUALCLEARDATA
#define SS_EVENT_VirtualClearData (SS_ISVB1 ? IEVENTVB1_SPREAD_VIRTUALCLEARDATA : \
                                IEVENT_SPREAD_VIRTUALCLEARDATA)
#endif
#ifndef SSE_NO_DATAADDNEW
#define SS_EVENT_DataAddNew    (SS_ISVB1 ? IEVENTVB1_SPREAD_DATAADDNEW : \
                                IEVENT_SPREAD_DATAADDNEW)
#endif
#ifndef SSE_NO_INSDEL
#define SS_EVENT_InsDel        (SS_ISVB1 ? IEVENTVB1_SPREAD_INSDEL : \
                                IEVENT_SPREAD_INSDEL)
#endif
#ifndef SSE_NO_ARROWPRESSED
#define SS_EVENT_ArrowPressed  (SS_ISVB1 ? IEVENTVB1_SPREAD_ARROWPRESSED : \
                                IEVENT_SPREAD_ARROWPRESSED)
#endif
#ifdef SS_V30
#ifndef SSE_NO_COMBOCLOSEUP
#define SS_EVENT_ComboCloseUp  (SS_ISVB1 ? IEVENTVB1_SPREAD_COMBOCLOSEUP : \
                                IEVENT_SPREAD_COMBOCLOSEUP)
#endif
#ifndef SSE_NO_COMBODROPDOWN
#define SS_EVENT_ComboDropDown (SS_ISVB1 ? IEVENTVB1_SPREAD_COMBODROPDOWN : \
                                IEVENT_SPREAD_COMBODROPDOWN)
#endif
#ifndef SSE_NO_COMBOSELCHANGE
#define SS_EVENT_ComboSelChange (SS_ISVB1 ? IEVENTVB1_SPREAD_COMBOSELCHANGE : \
                                IEVENT_SPREAD_COMBOSELCHANGE)
#endif
#ifndef SSE_NO_TEXTTIPFETCH
#define SS_EVENT_TextTipFetch   (SS_ISVB1 ? IEVENTVB1_SPREAD_TEXTTIPFETCH : \
                                IEVENT_SPREAD_TEXTTIPFETCH)
#endif
#ifndef SSE_NO_EDITCHANGE
#define SS_EVENT_EditChange     (SS_ISVB1 ? IEVENTVB1_SPREAD_EDITCHANGE : \
                                IEVENT_SPREAD_EDITCHANGE)
#endif
#endif // SS_V30

#ifdef SS_V35
#define SS_EVENT_BeforeUserSort     IEVENT_SPREAD_BEFOREUSERSORT
#define SS_EVENT_AfterUserSort      IEVENT_SPREAD_AFTERUSERSORT
#endif

extern PPROPINFO  SpreadProperties[IPROP_SPREAD_CNT + 1];
extern PPROPINFO  SpreadProperties1[IPROP_SPREAD_CNT + 1];
extern PPROPINFO  SpreadProperties2[IPROP_SPREAD_CNT + 1];

extern PEVENTINFO SpreadEventsVB1[IEVENTVB1_SPREAD_CNT + 1];

extern PEVENTINFO SpreadEvents[IEVENT_SPREAD_CNT + 1];
extern MODEL      ModelSpread;
extern MODELINFO  ModelInfoSpread;
extern MODEL      ModelSpreadVB1;
extern MODELINFO  ModelInfoSpreadVB1;
extern MODEL      ModelSpreadVB2;
extern MODELINFO  ModelInfoSpreadVB2;

/**********************
* Function Prototypes
**********************/

#if 0
extern BOOL FAR PASCAL           SSSetFreeze(HWND hWnd, short ColsFrozen,
                                             short RowsFrozen);
extern BOOL FAR PASCAL           SSGetFreeze(HWND hWnd, LPSHORT lpColsFrozen,
                                             LPSHORT lpRowsFrozen);
extern void FAR PASCAL           SSSetAutoSizeVisible(HWND hWnd,
                                                      short VisibleCols,
                                                      short VisibleRows);
extern void FAR PASCAL           SSGetAutoSizeVisible(HWND hWnd,
                                                      LPSHORT lpVisibleCols,
                                                      LPSHORT lpVisibleRows);
extern LPSS_CELLTYPE FAR PASCAL  SSSetTypeButton(HWND hWnd,
                                                 LPSS_CELLTYPE CellType,
                                                 long Style, LPSTR Text,
                                                 LPSTR Picture,
                                                 short nPictureType,
                                                 LPSTR PictureDown,
                                                 short nPictureDownType,
                                                 short ButtonType,
                                                 short nShadowSize,
                                                 LPSUPERBTNCOLOR lpColor);
extern void FAR PASCAL  SSSetProcessTab(HWND hWnd, BOOL fProcessTab);
extern BOOL FAR PASCAL  SSGetProcessTab(HWND hWnd);
extern BOOL FAR PASCAL  SSSetValue(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                   LPSTR lpData);
extern BOOL FAR PASCAL  SSSetValueRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                        SS_COORD Col2, SS_COORD Row2,
                                        LPSTR lpData);
extern short FAR PASCAL SSGetValue(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                   LPSTR lpBuffer);
extern short FAR PASCAL SSGetValueLen(HWND hWnd, SS_COORD Col, SS_COORD Row);
extern BOOL FAR PASCAL  SSClearData(HWND hWnd, SS_COORD Col, SS_COORD Row);
extern BOOL FAR PASCAL  SSClearDataRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                         SS_COORD Col2, SS_COORD Row2);
#endif

#ifdef _WIN64
extern LRESULT DLLENTRY   SpreadSheetProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam);
#else
extern LRESULT FAR PASCAL  SpreadSheetProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam);
#endif
/* vbprint.c declaration */
extern LONG             vbSpreadPrintAction(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread);

/* vbbound.c declaration */
LONG                    vbSpreadBoundProc(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, WORD Msg, WORD wParam, LONG lParam);
BOOL                    vbSpreadBoundClick(LPSPREADSHEET lpSS, LONG lObject, SS_COORD Row);
LONG                    vbSSBoundVQueryData(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd,
                                            LPSS_VQUERYDATA lpVQueryData);
BOOL                    vbSSBoundVClearData(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd,
                                            LPSS_VCLEARDATA lpVClearData);
BOOL                    VBSSBoundDel(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Row1,
                                     SS_COORD Row2);
BOOL                    VBSSBoundAdd(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Row1,
                                     SS_COORD Row2, BOOL fInsert);
BOOL                    VBSSBoundManualSave(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd,
                                            LPVBSPREAD lpSpread,
                                            SS_COORD Top, SS_COORD End,
                                            BOOL fSetCurrentRow);
BOOL                    VBSSBoundIsBound(LONG lObject, LPSPREADSHEET lpSS);
void                    VBSSBoundProcessPostAdd(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd,
                                                long lParam);
void                    VBSSBoundChangeMade(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Col,
                                            SS_COORD Row);
BOOL                    VBSSBoundSetColWidth(LPSPREADSHEET lpSS, HWND hWnd, LPVBSPREAD lpSpread);
ULONG  ssVBSetControlFlags(LONG lObject, ULONG mask, ULONG value, LPSPREADSHEET lpSS); //DBOCX
#ifdef SS_OCX
BOOL   vbSpreadSaveBoundRow(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Row, SS_COORD RowNew,
                          SS_COORD ColNew);
#else
BOOL   vbSpreadSaveBoundRow(LPSPREADSHEET lpSS, HCTL hCtl, HWND hWnd, SS_COORD Row,
                            SS_COORD RowNew, SS_COORD ColNew);
#endif //SS_OCX
/* vbsprea4.c declaration */
LONG                    FPx_TypePictPicture(BOOL fSetProp, HCTL hCtl,
                                            HWND hWnd, WORD Msg, WORD wParam,
                                            LONG lParam, LPVBSPREAD lpSpread);

void                    vbSSInit(HCTL hCtl, HWND hWnd);
LONG FAR PASCAL _export vbSpreadProc(HCTL hCtl, HWND hWnd, WORD Msg,
                                     WORD wParam, LONG lParam);
BOOL                    SetCellType(HWND hWnd, LPVBSPREAD lpSpread);
BOOL                    SetCellTypeProperties(HWND hWnd, LPVBSPREAD lpSpread,
                                              short Property, long lParam);
BOOL                    GetCellTypeProperties(HCTL hCtl, HWND hWnd,
                                              LPVBSPREAD lpSpread,
                                              short Property,
                                              long lParam);
ERR FAR PASCAL          SaveToFile(HWND hWnd, USHORT FileNum, BOOL fDataOnly);
BOOL FAR PASCAL         LoadFromFile(HWND hWnd, USHORT FileNum);
extern void FAR PASCAL  SSGetLockColor(HWND hWnd, LPCOLORREF lpBackground,
                                       LPCOLORREF lpForeground);
extern void FAR PASCAL  SSSetLockColor(HWND hWnd, COLORREF Background,
                                       COLORREF Foreground);
extern short            vbSSValCoords(LPVBSPREAD lpSpread, LPSS_COORD lpCol,
                                      LPSS_COORD lpRow);
extern short            vbSpreadSetError(short nErrNum);
extern void             SpreadInit(HWND hWnd);

extern void             SS_TypeGetPict(long lParam, HPIC hPic);
void                    vbSpreadSetUtopiaModeOn(HWND hWnd, LPVBSPREAD lpSpread,
                                                BOOL fValue);

void FAR PASCAL _export SpreadSetScrollArrows(HCTL hCtl,
                        HANDLE hTopUp, WORD wTopUpType,
                        HANDLE hTopDown, WORD wTopDownType,
                        HANDLE hBottomUp, WORD wBottomUpType,
                        HANDLE hBottomDown, WORD wBottomDownType,
                        HANDLE hLeftUp, WORD wLeftUpType,
                        HANDLE hLeftDown, WORD wLeftDownType,
                        HANDLE hRightUp, WORD wRightUpType,
                        HANDLE hRightDown, WORD wRightDownType);

void                    VBSSBoundFireDataColConfig(LONG lObject,
                                                   LPVBSPREAD lpSpread,
                                                   LPTSTR lpszFieldName,
                                                   SS_COORD Col,
                                                   USHORT usDataType, LPSPREADSHEET lpSS);
BOOL                    VBSSBoundFireDataFill(LPSPREADSHEET lpSS, LONG lObject, LPVBSPREAD lpSpread,
                                              SS_COORD Col, SS_COORD Row,
                                              BOOL fGetData, USHORT usDataType,
                                              long lBookMark, long lDataCol);
void                    VBSSBoundFinishClipData(HWND hWnd, LPSPREADSHEET lpSS,
                                                SS_COORD Row1, SS_COORD Row2,
                                                SS_COORD ColCnt);
HSZ                     VBSSBoundGetFieldName(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd,
                                              SS_COORD Col);
void                    SS_DataRefresh(LPSPREADSHEET lpSS, long lObject);

void                    SetDataCellTypes3(HWND hWnd, LPSPREADSHEET lpSS, SS_COORD Col, USHORT usDataType);
void                    SSDataRefresh(HCTL hCtl, HWND hWnd);

BOOL                    VBSSBmpLoad(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,
                                    HPSTR lpBmp, long lSize);
HLSTR                   VBSSBmpSave(LPSPREADSHEET lpSS, HBITMAP hBitmap, HPALETTE hPal,
                                    long lStyle, SS_COORD Col, SS_COORD Row);
void                    InitFunctionArray(void);
void                    VBSSPrintPreviewGetInfo(HCTL hCtl, HWND hWnd,
                                                LPPRVW_INFO lpPrvwInfo);
void                    VBSSPrintPreviewPrintPage(HCTL hCtl, HWND hWnd,
                                                  LPPRVW_PAGE lpPrvwInfo);
void                    VBSSPrintPreviewSetMargins(HCTL hCtl, HWND hWnd,
                                                   LPPRVW_MARGINS lpPrvwInfo);
LONG                    VBSS_Print(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread,
                                   WORD wAction, HDC hDCPreview,
                                   LPRECT lpRectPreview, long lXtra,
                                   LPSS_PRINTPAGECALC lpPrintPageCalc);

#define NOTUSED(PropName, PropNum) static PROPINFO PropName = {PropNum, DT_SHORT | PF_fNoShow};


#define VBSSVALCOLROW(lpSpread)                        \
   {                                                   \
   short nRet;                                         \
   if ((nRet = vbSSValCoords(lpSpread, &lpSpread->Col, \
                            &lpSpread->Row)) != 0)     \
      return (nRet);                                   \
   }

#define VBSSVALCOL2ROW2(lpSpread)                      \
   {                                                   \
   short nRet;                                         \
   if ((nRet = vbSSValCoords(lpSpread, &lpSpread->Col2,\
                            &lpSpread->Row2)) != 0)    \
      return (nRet);                                   \
   }

#define VBSSVALCOL(lpSpread)                           \
   {                                                   \
   short nRet;                                         \
   if ((nRet = vbSSValCoords(lpSpread, &lpSpread->Col, \
                            NULL)) != 0)               \
      return (nRet);                                   \
   }

#define VBSSVALROW(lpSpread)                           \
   {                                                   \
   short nRet;                                         \
   if ((nRet = vbSSValCoords(lpSpread, NULL,           \
                            &lpSpread->Row)) != 0)     \
      return (nRet);                                   \
   }

#define VBSSVALCOL2(lpSpread)                          \
   {                                                   \
   short nRet;                                         \
   if ((nRet = vbSSValCoords(lpSpread, &lpSpread->Col2,\
                             NULL)) != 0)              \
      return (nRet);                                   \
   }

#define VBSSVALROW2(lpSpread)                          \
   {                                                   \
   short nRet;                                         \
   if ((nRet = vbSSValCoords(lpSpread, NULL,           \
                            &lpSpread->Row2)) != 0)    \
      return (nRet);                                   \
   }

#ifdef SS_NOCELLFONTCOLOR
#define LIMIT_TO_COLROW if (lpSpread->Col != -1 && lpSpread->Row != -1) \
                           lpSpread->Row = -1
#else
#define LIMIT_TO_COLROW
#endif

#endif
