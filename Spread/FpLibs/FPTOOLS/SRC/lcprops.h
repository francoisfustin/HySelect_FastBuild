/************
* lcprops.h
************/

#define FP_ID(dispid)  (dispid + 1)      // OCX disp ID's
#define LB_HID(dispid) (dispid + 5000)   // OCX list help ID's
#define CB_HID(dispid) (dispid + 6000)   // OCX combo help ID's

#define LBPROP_VBX_CTLNAME                 0
#define LBPROP_VBX_INDEX                   1
#define LBPROP_BackColor                   2
#define LBPROP_ForeColor                   3
#define LBPROP_VBX_LEFT                    4
#define LBPROP_VBX_TOP                     5
#define LBPROP_VBX_WIDTH                   6
#define LBPROP_VBX_HEIGHT                  7
#define LBPROP_VBX_MOUSEPOINTER            8
#define LBPROP_VBX_FONTNAME                9
#define LBPROP_VBX_FONTBOLD                10
#define LBPROP_VBX_FONTITALIC              11
#define LBPROP_VBX_FONTSTRIKE              12
#define LBPROP_VBX_FONTUNDER               13
#define LBPROP_VBX_FONTSIZE                14
#define LBPROP_VBX_TABINDEX                15
#define LBPROP_VBX_TABSTOP                 16
#define LBPROP_VBX_DRAGICON                17
#define LBPROP_VBX_DRAGMODE                18
#define LBPROP_VBX_ENABLED                 19
#define LBPROP_VBX_PARENT                  20
#define LBPROP_VBX_TAG                     21
#define LBPROP_VBX_VISIBLE                 22
#define LBPROP_VBX_HELPCONTEXTID           23
#define LBPROP_VBX_DATASOURCE              24
#define LBPROP_VBX_DATAFIELD               25
#define LBPROP_VBX_DATACHANGED             26
#define LBPROP_ScrollBars                  27   // -Not Used any more V1.0
#define LBPROP_Col                         28
#define LBPROP_Row                         29
#define LBPROP_Text                        30
#define LBPROP_List                        31
#define LBPROP_Columns                     32
#define LBPROP_Sorted                      33
#define LBPROP_FieldType                   34   // -Not Used any more V1.0
#define LBPROP_TopIndex                    35
#define LBPROP_LineHighlightColor          36   // -Not Used any more V1.5
#define LBPROP_LineShadowColor             37   // -Not Used any more V1.5
#define LBPROP_LineSolidColor              38   // -Not Used any more V1.5
#define LBPROP_LineStyleH                  39   // -Not Used any more V1.5
#define LBPROP_LineStyleV                  40   // -Not Used any more V1.5
#define LBPROP_LineWidth                   41
#define LBPROP_Sel3DStyle                  42   // -Not Used any more V1.5
#define LBPROP_SelBackColor                43   // -Not Used any more V1.5
#define LBPROP_SelForeColor                44   // -Not Used any more V1.5
#define LBPROP_SelHighlightColor           45   // -Not Used any more V1.5
#define LBPROP_SelShadowColor              46   // -Not Used any more V1.5
#define LBPROP_Sel3DWidth                  47   // -Not Used any more V1.5
#define LBPROP_SelDrawFocusRect            48
#define LBPROP_HeaderBackColor             49   // -Not Used any more V1.5
#define LBPROP_HeaderFontBold              50   // -Not Used any more V1.5
#define LBPROP_HeaderFontItalic            51   // -Not Used any more V1.5
#define LBPROP_HeaderFontName              52   // -Not Used any more V1.5
#define LBPROP_HeaderFontSize              53   // -Not Used any more V1.5
#define LBPROP_HeaderFontStrike            54   // -Not Used any more V1.5
#define LBPROP_HeaderFontUnder             55   // -Not Used any more V1.5
#define LBPROP_HeaderForeColor             56   // -Not Used any more V1.5
#define LBPROP_HeaderHeight                57   // -Not Used any more V1.5
#define LBPROP_HeaderShow                  58   // -Not Used any more V1.5
#define LBPROP_Header3DStyle               59   // -Not Used any more V1.5
#define LBPROP_Header3DWidth               60   // -Not Used any more V1.5
#define LBPROP_ColAlignH                   61   // -Not Used any more V1.5
#define LBPROP_ColDataField                62   // VB and OCX Only
#define LBPROP_ColDataFieldType            63   // -Not Used any more
#define LBPROP_ColFormat                   64   // VB and OCX Only
#define LBPROP_ColHeaderText               65
#define LBPROP_ColHeaderAlignH             66   // -Not Used any more V1.5
#define LBPROP_ColText                     67
#define LBPROP_ColList                     68
#define LBPROP_ColumnSeparatorChar         69
#define LBPROP_ColumnSearch                70
#define LBPROP_ColumnWidthScale            71
#define LBPROP_ColHide                     72
#define LBPROP_ColWidth                    73
#define LBPROP_ColSorted                   74
#define LBPROP_ColSortSeq                  75
#define LBPROP_ListCount                   76
#define LBPROP_ListIndex                   77
#define LBPROP_RowHeight                   78
#define LBPROP_ItemData                    79
#define LBPROP_MultiSelect                 80
#define LBPROP_WrapList                    81
#define LBPROP_WrapWidth                   82
#define LBPROP_Selected                    83
#define LBPROP_SelMax                      84
#define LBPROP_Action                      85
#define LBPROP_AutoSearch                  86
#define LBPROP_SearchIndex                 87
#define LBPROP_SearchMethod                88
#define LBPROP_SearchText                  89
#define LBPROP_NewIndex                    90
#define LBPROP_About                       91
#define LBPROP_List3DText                  92
#define LBPROP_List3DTextHighlightColor    93
#define LBPROP_List3DTextShadowColor       94
#define LBPROP_List3DTextOffset            95
#define LBPROP_Header3DText                96   // -Not Used any more V1.5
#define LBPROP_Header3DTextHighlightColor  97   // -Not Used any more V1.5
#define LBPROP_Header3DTextShadowColor     98   // -Not Used any more V1.5
#define LBPROP_Header3DTextOffset          99   // -Not Used any more V1.5
#define LBPROP_VirtualMode                 100
#define LBPROP_VRowCount                   101
#define LBPROP_DataSync                    102  // VB Only
#define LBPROP_SelCount                    103
#define LBPROP_VisibleRows                 104
#define LBPROP_ThreeDInsideStyle           105
#define LBPROP_ThreeDInsideHighlightColor  106
#define LBPROP_ThreeDInsideShadowColor     107
#define LBPROP_ThreeDInsideWidth           108
#define LBPROP_ThreeDOutsideStyle          109
#define LBPROP_ThreeDOutsideHighlightColor 110
#define LBPROP_ThreeDOutsideShadowColor    111
#define LBPROP_ThreeDOutsideWidth          112
#define LBPROP_ThreeDFrameWidth            113
#define LBPROP_BorderStyle                 114
#define LBPROP_BorderColor                 115
#define LBPROP_BorderWidth                 116
#define LBPROP_OnFocusInvert3D             117
#define LBPROP_OnFocusShadow               118
#define LBPROP_DropShadowColor             119
#define LBPROP_DropShadowWidth             120
#define LBPROP_ThreeDOnFocusInvert         121
#define LBPROP_ThreeDFrameColor            122
#define LBPROP_Appearance                  123
#define LBPROP_BorderDropShadow            124
#define LBPROP_BorderDropShadowColor       125
#define LBPROP_BorderDropShadowWidth       126
#define LBPROP_ScrollHScale                127
#define LBPROP_ScrollHInc                  128
#define LBPROP_ColsFrozen                  129
#define LBPROP_ScrollBarV                  130
#define LBPROP_NoIntegralHeight            131
#define LBPROP_ColMultiLine                132  // -Not Used any more V1.5
#define LBPROP_ColBackColor                133  // -Not Used any more V1.5
#define LBPROP_ColForeColor                134  // -Not Used any more V1.5
#define LBPROP_RowBackColor                135  // -Not Used any more V1.5
#define LBPROP_RowForeColor                136  // -Not Used any more V1.5
#define LBPROP_HighestPrecedence           137
#define LBPROP_InsertRow                   138
#define LBPROP_ColPicture                  139  // -Not Used any more V1.5
#define LBPROP_ColPictureSel               140  // -Not Used any more V1.5
#define LBPROP_ColPictureAlignH            141  // -Not Used any more V1.5
#define LBPROP_AllowColResize              142
#define LBPROP_AllowColDragDrop            143
#define LBPROP_ReadOnly                    144
#define LBPROP_ColLock                     145
#define LBPROP_ColLockResize               146
#define LBPROP_RowPicture                  147  // -Not Used any more V1.5
#define LBPROP_RowPictureSel               148  // -Not Used any more V1.5
#define LBPROP_RowPictureCol               149  // -Not Used any more V1.5
#define LBPROP_RowPictureAlignH            150  // -Not Used any more V1.5
#define LBPROP_VScrollSpecial              151
#define LBPROP_VScrollSpecialType          152
#define LBPROP_MouseOverCol                153
#define LBPROP_MouseOverRow                154
#define LBPROP_MouseOverColHeader          155
#define LBPROP_EnableKeyEvents             156
#define LBPROP_EnableMouseEvents           157
#define LBPROP_EnableTopChangeEvent        158
#define LBPROP_DataAutoHeadings            159  // VB Only
#define LBPROP_DataAutoSizeCols            160  // VB Only
#define LBPROP_DataBookmark                161  // VB Only
#define LBPROP_DataSourcehWnd              162  // VB Only
#define LBPROP_ColSortDataType             163
#define LBPROP_SortState                   164
#define LBPROP_SearchIgnoreCase            165
#define LBPROP_VBX_COLDESIGNER             166
#define LBPROP_ScrollBarH                  167
#define CBPROP_DataSourceList              168  // VB Only
#define CBPROP_DataFieldList               169  // VB Only
#define CBPROP_ColumnEdit                  170
#define CBPROP_ColumnBound                 171  // VB Only
#define CBPROP_Style                       172
#define CBPROP_MaxDrop                     173
#define CBPROP_ListWidth                   174
#define CBPROP_SelStart                    175
#define CBPROP_SelLength                   176
#define CBPROP_SelText                     177
#define CBPROP_EditHeight                  178
#define CBPROP_GrayAreaColor               179
#define CBPROP_ListLeftOffset              180
#define CBPROP_ApplyTo                     181
#define CBPROP_ListDown                    182
#define CBPROP_ComboGap                    183
#define CBPROP_DataSourcehWndList          184  // VB Only
#define CBPROP_MaxEditLen                  185
#define LBPROP_VirtualPageSize             186
#define LBPROP_VirtualPagesAhead           187
#define LBPROP_hWnd                        188
#define LBPROP_ExtendCol                   189
#define LBPROP_LineColor                   190
#define LBPROP_Line3DLight                 191
#define LBPROP_Line3DDark                  192
#define LBPROP_Line3DWidth                 193
#define LBPROP_LineStyle                   194
#define LBPROP_LineApplyTo                 195
#define LBPROP_ListApplyTo                 196
#define LBPROP_Grp                         197
#define LBPROP_GrpHide                     198
#define LBPROP_GrpHeaderText               199
#define LBPROP_GrpWidth                    200
#define LBPROP_GrpParentGroup              201
#define LBPROP_GrpLockResize               202
#define LBPROP_ColParentGroup              203
#define LBPROP_ColLevel                    204
#define LBPROP_ColLevelHeight              205
#define LBPROP_Groups                      206
#define LBPROP_ColumnLevels                207
#define LBPROP_ListGrayAreaColor           208
#define LBPROP_ColMerge                    209
#define LBPROP_RowMerge                    210
#define LBPROP_MultiLine                   211
#define LBPROP_AlignH                      212
#define LBPROP_AlignV                      213
#define LBPROP_Picture                     214
#define LBPROP_PictureSel                  215
#define LBPROP_PictureAlignH               216
#define LBPROP_PictureAlignV               217
#define LBPROP_GroupHeaderHeight           218
#define LBPROP_GroupHeaderShow             219
#define LBPROP_AllowGrpResize              220
#define LBPROP_AllowGrpDragDrop            221
#define LBPROP_TextOrientation             222
#define LBPROP_JoinID                      223
#define LBPROP_ColPos                      224
#define LBPROP_ColPosInParent              225
#define LBPROP_GrpPos                      226
#define LBPROP_GrpPosInParent              227
#define LBPROP_MergeAdjustView             228
#define LBPROP_ColumnHeaderShow            229
#define LBPROP_ColumnHeaderHeight          230
#define LBPROP_MouseOverGrp                231
#define LBPROP_MouseOverGrpHeader          232
#define LBPROP_ColID                       233
#define LBPROP_GrpID                       234
#define LBPROP_ColName                     235
#define LBPROP_GrpName                     236
#define LBPROP_ColFromID                   237
#define LBPROP_GrpFromID                   238
#define LBPROP_ColFromName                 239
#define LBPROP_GrpFromName                 240
#define LBPROP_GrpsFrozen                  241
#define LBPROP_BorderGrayAreaColor         242
#define LBPROP_PropBrowser                 243  //$PBRW
#define LBPROP_FontEmpty                   244
#define LBPROP_InternalFont                245  //$PBRW
#define LBPROP_NextSel                     246
#define LBPROP_ExtendRow                   247
#define CBPROP_MouseOverArea               248

#ifndef LP30

#if defined(FP_VB) || (defined(LIST_STATIC) && !defined(WIN32))
#define LCPROP_PropCnt                     249
#else
#define LCPROP_VRowPos                     249
#define LCPROP_PropCnt					   250
#endif

#else // LP30:

#define LCPROP_RowHide					   249
#define LCPROP_TextEllipses				   250
#define LCPROP_EnableClickEvent			   251
#define CBPROP_ListPosition				   252
#define CBPROP_ButtonThreeDAppearance	   253
//#ifndef NO_MFC	// this is the max for properties in the VBX control -scl
//#define LCPROP_PropCnt					   254
//#else
#define LCPROP_VRowPos                     254
#define LCPROP_Redraw					   255
#define CBPROP_AutoSearchFill			   256
#define CBPROP_AutoSearchFillDelay		   257
#define CBPROP_EditMarginLeft			   258
#define CBPROP_EditMarginTop	           259
#define CBPROP_EditMarginRight			   260
#define CBPROP_EditMarginBottom            261
#define LCPROP_ResizeRowToFont			   262
#define LCPROP_TextTip					   263
#define LCPROP_TextTipDelay				   264
#define LCPROP_TextTipMultiline			   265
#define LCPROP_TextTipBackColor			   266
#define LCPROP_TextTipForeColor            267
#define LCPROP_IsHScrollVisible			   268
#define LCPROP_IsVScrollVisible			   269
#define LCPROP_DragColLevel				   270
#define LCPROP_DragParentGrp  		       271
#define LCPROP_DragPosInParent			   272
#define CBPROP_AutoMenu               273
#define CBPROP_EditAlignH             274
#define CBPROP_EditAlignV             275
#define LCPROP_TextTipFont				   276
#define LCPROP_MouseIcon				   277
//#define CBPROP_Text						   271
#define LCPROP_OLEDragMode				   278
#define LCPROP_OLEDropMode				   279
#define CBPROP_AllowAnimate            280
#define LCPROP_PropCnt                     281
//#define LCPROP_MouseIcon				   255
//#define LCPROP_OLEDragMode				   256
//#define LCPROP_OLEDropMode				   257
//#define CBPROP_Text						   258
//#define LCPROP_PropCnt                     259
//#endif

#define LC_OLDVER_PROPCNT 250

#endif // LP30

//Added for DLL support
#ifdef FP_DLL
#define LBPROP_Enabled                     500
#define LBPROP_FontBold                    501
#define LBPROP_FontItalic                  502
#define LBPROP_FontName                    503
#define LBPROP_FontSize                    504
#define LBPROP_FontStrike                  505
#define LBPROP_FontUnder                   506
#define LBPROP_HelpContextID               507
#define LBPROP_TabStop                     508
#define LBPROP_Visible                     509
#endif   

#ifdef FP_ADB
#define LBPROP_DataMember					510
#define CBPROP_DataMemberList				511
#endif

#if defined(LP30) && !defined(FP_DLL)
//#define DISPID_OLEDRAGMODE					264
//#define DISPID_OLEDROPMODE					265
//#define DISPID_OLEDRAG						266
//#define DISPID_OLECOMPLETEDRAG				267
//#define DISPID_OLEDRAGDROP					268
//#define DISPID_OLEDRAGOVER					269
//#define DISPID_OLEGIVEFEEDBACK				270
//#define DISPID_OLESETDATA					271
//#define DISPID_OLESTARTDRAG					272
#define DISPID_TEXTTIPFONT					277
#define DISPID_OLEDRAGMODE					278
#define DISPID_OLEDROPMODE					279
#define DISPID_OLEDRAG						280
#define DISPID_OLECOMPLETEDRAG				281
#define DISPID_OLEDRAGDROP					282
#define DISPID_OLEDRAGOVER					283
#define DISPID_OLEGIVEFEEDBACK				284
#define DISPID_OLESETDATA					285
#define DISPID_OLESTARTDRAG					286

#define OLEDRAGMODE_HELPCONTEXT				LB_HID(LCPROP_OLEDragMode)
#define OLEDROPMODE_HELPCONTEXT				LB_HID(LCPROP_OLEDropMode)
#define OLEDRAGOVERCONTSTANTS_HELPCONTEXT	3602
#define CLIPFORMATCONSTANTS_HELPCONTEXT		10002
#define OLECOMPLETEDRAG_HELPCONTEXT			3600
#define OLEDRAGDROP_HELPCONTEXT				3601
#define OLEDRAGOVER_HELPCONTEXT				3602
#define OLEGIVEFEEDBACK_HELPCONTEXT			3603
#define OLESETDATA_HELPCONTEXT				3604
#define OLESTARTDRAG_HELPCONTEXT			3605
#define FPDATAOBJECT_HELPCONTEXT			3606
#define FPDATAOBJECTFILES_HELPCONTEXT		3607
#endif // LP30
