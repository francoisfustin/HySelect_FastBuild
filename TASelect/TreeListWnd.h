//*****************************************************************************
//*
//*
//*		TreeListWnd.h
//*
//*
//*****************************************************************************
#ifndef 	__TREELISTWND_H__
#define 	__TREELISTWND_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include	<commctrl.h>

#ifndef		GWLP_USERDATA
#define 	GWLP_USERDATA		GWL_USERDATA
#endif
#ifndef 	GWLP_WNDPROC
#define 	GWLP_WNDPROC		GWL_WNDPROC
#endif
#ifndef	   _WIN64
#ifndef		SetWindowLongPtr
#define		SetWindowLongPtr	SetWindowLong
#endif
#ifndef		GetWindowLongPtr
#define		GetWindowLongPtr	GetWindowLong
#endif
#ifndef		DWORD_PTR
#define		DWORD_PTR			DWORD
#endif
#ifndef		LONG_PTR
#define		LONG_PTR			LONG
#endif
#endif
#ifdef		UNICODE
#define 	str_len				(unsigned)wcslen
#define 	str_cmp				wcscmp
#define 	str_ncpy			wcsncpy
#define 	str_ncmp			wcsncmp
#define 	str_icmp		   _wcsicmp
#else
#define 	str_len				(unsigned)strlen
#define 	str_cmp				strcmp
#define 	str_ncpy			strncpy
#define 	str_ncmp			strncmp
#define 	str_icmp		   _stricmp
#endif
#ifndef		WM_MOUSEWHEEL
#define		WM_MOUSEWHEEL		0x020A
#endif
#ifndef		WHEEL_DELTA
#define		WHEEL_DELTA			120
#endif
#ifndef		MAX_COLUMNS
#define 	MAX_COLUMNS			32
#endif
#define		MAX_COLORS			16
#define 	EN_SETTEXT			0x1000
#define 	EN_RETURN			0x1578
#define 	EN_ESCAPE			0x1579
#define 	ID_TOOLTIPCHECK		0x3912
#define 	SORT_NOUPDATE		1234567
#define 	VK_ISACHAR			0x01000000
#define 	FIRST_LINE			0xFFFFFFFE
#define 	I_CCB				I_CHILDRENCALLBACK
#define 	U(h)				((unsigned)(h))
#define 	THEMEIMGLIST		((HIMAGELIST)1)
#define 	GetHandle(h)		((TreeListData*)GetWindowLongPtr(h,0))
#define 	TVIF_ALL			(TVIF_CHILDREN|TVIF_HANDLE|TVIF_IMAGE|TVIF_PARAM|TVIF_SELECTEDIMAGE|TVIF_STATE|TVIF_TEXT)
#define 	UNLOCK(d)			ReleaseSemaphore(d->hSem,1,NULL)
#define 	LOCK(d)				WaitForSingleObject(d->hSem,INFINITE)
#define 	TVIS_EDIT(m)		((1<<m)&((1<<TVAX_EDIT)|(1<<TVAX_COMBO)|(1<<TVAX_STEPED)|(1<<TVAX_CHECKED)))
#define 	TVIS_BASEFLAGS		(TVIS_EXPANDED|TVIS_EXPANDEDONCE|TVIS_EXPANDPARTIAL|TVIS_SELECTED)
#define 	TVIS_TRACKED		(TVIX_TRACKED<<16)
#define 	TVIS_BKCOLOR		(TVIX_BKCOLOR<<16)
#define 	TVIS_FOCUSED		(TVIX_FOCUSED<<16)
#define 	TVIS_TEXTCOLOR		(TVIX_TEXTCOLOR<<16)
#define 	TVC_ONLYFOCUS		TVIF_ONLYFOCUS
#define 	TVC_UNSELECT		0x4000
#define 	TVC_DESELECT		0x8000
#define 	DEFAULT_IDENT		19
#define 	DEFAULT_SHIFT		7
#ifndef		BPBF_COMPATIBLEBITMAP
#define 	BPBF_COMPATIBLEBITMAP	0
#endif
#ifndef		TVP_GLYPH
#define		TVP_GLYPH			2
#endif
#ifndef		GLPS_CLOSED
#define		GLPS_CLOSED			1
#endif
#ifndef		GLPS_OPENED
#define		GLPS_OPENED			2
#endif
#ifndef		BP_CHECKBOX
#define		BP_CHECKBOX			3
#endif
#ifndef		CBS_UNCHECKEDNORMAL
#define		CBS_UNCHECKEDNORMAL 1
#endif
#ifndef		CBS_CHECKEDNORMAL
#define		CBS_CHECKEDNORMAL	5
#endif


#define 	TVAX_NONE			(TVAE_NONE   >>TVAE_MODEPOS)// Kein atomatisches editieren
#define 	TVAX_EDIT			(TVAE_EDIT	 >>TVAE_MODEPOS)// Atomatisches editieren mit Edit-Fenster
#define 	TVAX_COMBO			(TVAE_COMBO	 >>TVAE_MODEPOS)// Atomatisches editieren mit ComboBox
#define 	TVAX_CBLIST			(TVAE_CBLIST >>TVAE_MODEPOS)// Atomatisches editieren mit ComboListBox
#define 	TVAX_STEP			(TVAE_STEP	 >>TVAE_MODEPOS)// Einzelnes Weiterschalten mit Enter
#define 	TVAX_STEPED			(TVAE_STEPED >>TVAE_MODEPOS)// Einzelnes Weiterschalten mit Enter und Edit
#define 	TVAX_CHECK			(TVAE_CHECK	 >>TVAE_MODEPOS)// Atomatisches editieren mit CheckBox
#define 	TVAX_CHECKED		(TVAE_CHECKED>>TVAE_MODEPOS)// Atomatisches editieren mit CheckBox und Edit

#define 	TVIX_VARBUTTON		0x01						// Set buttons are not fixed
#define 	TVIX_HASBUTTON		0x02						// The entry has a button
#define 	TVIX_HASIMAGE		0x04						// The entry has an icon
#define 	TVIX_TRACKED		0x08						// The entry under the cursor
#define 	TVIX_TEXTCOLOR		0x10						// The entry has its own text color
#define 	TVIX_BKCOLOR		0x20						// The entry has its own background color
#define 	TVIX_FOCUSED		0x40						// The entry has an input focus

typedef int (CALLBACK *PFNTVCOMPAREEX)(HWND hWnd,HTREEITEM hItem1,HTREEITEM hItem2,LPARAM lParam1,LPARAM lParam2,LPARAM lParam);
typedef int (CALLBACK *PFNTVSORTEX   )(HWND hWnd,HTREEITEM hItem ,LPCTSTR pTextItem,LPCTSTR pTextInsert,LPARAM lParamItem,LPARAM lParamInsert);

typedef struct
	{
	HTREEITEM       hParent;
	PFNTVCOMPAREEX  lpfnCompare;
	LPARAM          lParam;
	}TVSORTEX;

typedef struct
	{
	UINT			uFlags;
	UINT			uColumn;
	UINT			uState;
	UINT			uStateMask;
	LPARAM			lParam;
	LPCTSTR			pText;
	}TVFIND;

typedef struct
	{
	NMHDR			hdr;
	WORD			wVKey;
	WORD			wScan;
	UINT			flags;
	}TV_KEYDOWN_EX;

typedef struct 
	{
	NMHDR			hdr;
	TVITEM			item;
	UINT			uAction;
	UINT			uHeight;
	UINT			uMaxEntries;
	LPCTSTR			pTextEntries;
	LPCTSTR		   *pTextList;
	POINT			ptAction;
	}TV_STARTEDIT;

typedef struct
	{
	LPARAM		lParam;										// Is the LPARAM value for the entry
	LPTSTR		pText;										// Pointer to text Tree
	UINT		uState;										// Condition of entry
	UINT		uStateEx;									// TA: to save extended state.
	int			iImage;										// Is the number of icons to be displayed
	int			iSelectedImage; 							// Is the number of icons to be displayed (if selected)
	unsigned	uShowPos;									// If the position is visible in the list (0 = invisible)
	unsigned	uFirstChild;								// The number of the first child-entry is (0 = none)
	unsigned	uLastChild;									// Number of the last child-entry is (0 = none)
	unsigned	uPrevItem;									// Number of the previous entry is (0 = none)
	unsigned	uNextItem; 									// Number of the next entry is (0 = none)
	unsigned	uParent; 									// Is the number of the parent entry (0 = root)
	unsigned 	uLevel;										// Is the level of entry (0 = root)
	int 		iTextPixels;								// Is the width of the text in pixels
	WORD		uTextSize;									// Length of text in characters
	BYTE		bCallback;									// Are bits for callbacks
	BYTE		bFlags;										// Other flags
	COLORREF	uColorText;									// Special text color
	COLORREF	uColorBk;									// Special Background Color
	}BaseItem;

typedef struct
	{
	LPTSTR		pText;										// Zeiger auf Tree-Text
	UINT		uState;										// Condition of entry
	int			iImage;										// Ist die Nummer des an zu zeigenden Icons
	int 		iTextPixels;								// Ist die Breites des Textes in Pixel
	WORD		uTextSize;									// Länge des Textes in Zeichen
	BYTE		bCallback;									// Sind Bits für Callbacks
	BYTE		bFlags;										// Diverse Flags
	COLORREF	uColorText;									// Spezielle Textfarbe
	COLORREF	uColorBk;									// Spezielle Hintergrundfarbe
	}ExtraItem;

typedef struct
	{
	void 	   *pCbData;									// Daten für das Autoeditieren
	INT  	    iCbIcon;									// Startoffset für in Iconliste für Autoedit
	short 		sSize;										// Ist die soll Breite einer Spalte
	short 		sReal;										// Ist die wirkliche Breite einer Spalte
	short 		sMin;										// Ist die Ausrichtung einer Spalte
	BYTE		bMinEx;										// Darf die Spaltenbreite auch nicht vom User unterschritten werden
	BYTE		bWeight;									// Gewicht der variable Spalten
	BYTE  	    bAlign;										// Text ausrichtung für die Spalten
	BYTE  	    bEdit;										// Automaisches Editiern einer Spalte (siehe TVAE_???>>7)
	BYTE  	    bFlags;										// Automaisches Editiern einer Spalte (siehe TVAE_???)
	BYTE  	    bEnable;									// Automaisches einer mit Statebits aktivieren
	BYTE  	    bCbSize;									// Maximale Anzahl der Einträge in der Datenliste
	BYTE  	    bCbChar;									// Trennzeichnen für die Datenliste
	BYTE  	    bFixed;										// Ist die Spalte fixiert
	BYTE  	    bMark;										// Ist die Spalte markiert
	BYTE  	    bDummy[32-20-sizeof(void*)];				// Füllbytes auf 32 Grenze
	}ColumnData;

typedef struct
	{
	HWND		hWnd;										// Window handle
	HANDLE		hSem;										// Zugriffssemaphore
	LPVOID		hTheme;										// Handle für benutztes Thema (TREELIST)
	LPVOID		hThemeBt;									// Handle für benutztes Thema (BUTTON)
	WNDPROC		pProcId3;									// Fenster Funktion für ID3 Fenster
	HIMAGELIST	hStates;									// Handle of the icon list for States and overlay
	HIMAGELIST	hImages;									// Handle of the icon list
	HIMAGELIST	hChecks;									// Handle of the icon list for the check boxes in the columns
	HIMAGELIST	hSubImg;									// Handle of the icon list for the columns
	HFONT		hFontN;										// If the normal font
	HFONT		hFontB;										// If the bold font
	HFONT		hFontL;										// The last assigned font
	HFONT		hFontT;										// If the font for tooltips
	HWND		hEdit;										// Handle of the edit window
	HWND		hHeader;									// Handle to the window header
	HWND		hToolTip;									// Handle of the tool tip window
	WNDPROC		pToolProc;									// Stop function of the tooltip window
	COLORREF	uColors[MAX_COLORS];						// 0 = background color 1 = color 2 = Abwechselnte color for text color separation lines 3 =
	int			iFontHeight;								// If the height of the font
	int			iFontLine;									// If to underline the position of the line
	int			iFontOff;									// If the position is shifted horizontally to the text
	int			iStatesMode;								// The image hStates list was generated for the checkboxes
	int			iStatesXsize;								// Width of the States and overlay icons
	int			iStatesYsize;								// Height of the States and overlay icons
	int			iChecksMode;								// The image hChecks list was generated for the checkboxes
	int			iChecksXsize;								// Width of the States and overlay icons
	int			iChecksYsize;								// Height of the States and overlay icons
	int			iImagesXsize;								// Width of the icons
	int			iImagesYsize;								// Height of Icons
	int			iSubImgMode;								// The image list is not subimg hImages the list
	int			iSubImgXsize;								// Width of the icons
	int			iSubImgYsize;								// Height of Icons
	int			iRowHeight;									// If the height of a
	int			iAllWeight;									// The weight of all variable columns
	int			iVarSize;									// If the width of all variable columns
	int			iFixSize;									// If the width of all columns fixed
	int			iIndent;									// Indent the Kindereintäge
	int			iShift;										// Indentation of the vertical lines
	int			iAutoAdd;									// Offset zum Open-Icon  für TVS_EX_AUTOEXPANDICON
	int			iMaxSizeX;									// The size of the widest visible entry
	unsigned	uItemPosCount;								// Number of visible items
	unsigned   *pItemPos;									// List of the offsets of the visible entries
	BaseItem  **pTreeItems;									// Pointer to item pointer
	ExtraItem **pExtraItems[MAX_COLUMNS-1];					// Pointer to the column entries
	unsigned	uTreeItemsMax;								// Size of the list of available items (alociert larger by 1)
	unsigned	uTreeItemsCount;							// Number of entries available
	unsigned	uNextSeachPos;								// Next position to find spare entries
	unsigned	uUserDataSize;								// Is the size of user data in an entry
	unsigned	uFirstChild;								// The number of the first child-entry is (0 = none)
	unsigned	uLastChild;									// Number of the last child-entry is (0 = none)
	unsigned	uSingleSel;									// Ist die Nummer des gewählten Eintrages (bei Checkboxen)
	unsigned	uScrollX;									// Current Y scroll position
	unsigned	uScrollY;									// Current X-scroll position

	unsigned	uSizeX;										// Current X-window-size
	unsigned	uSizeY;										// Current Y window size
	unsigned	uSizeYsub;									// Current Y window size without header
	unsigned	uStyle;										// If the style of the window aktuele
	unsigned	uStyleEx;									// Advanced flag Sytle (see TVS_EX_?)
	unsigned	uStartPixel;								// If the Y-Koortinate in the first entry begins
	unsigned	uMaxEnties;									// Number of visible entries (including semi-visible)
	unsigned	uPageEnties;								// Number of visible entries (excluding semi-visible)
	unsigned	uColumnCount;								// Number of columns
	unsigned	uColumnCountVar;							// Number of variables columns
	unsigned	uSelectedCount;								// Number of the selected entries
	unsigned	uSelectedBase;								// If the entry is selected from the
	unsigned	uSelectedItem;								// If the entry is currently selected
	unsigned	uSelectedSub;								// If the column is currently selected
	unsigned	uFocusItem;									// If the entry of an empty focus has
	unsigned	uFocusSub;									// If the column has a blank Focus
	unsigned	uToolTipItem;								// If the ToolTip entry is currently selected
	unsigned	uToolTipShow;								// If the time delay at 500 ms intervals for the tooltip
	unsigned	uToolTipSub;								// If the ToolTip column is currently selected
	POINT		sToolTipPos;								// If the global coordinate of the ToolTips
	unsigned	uEditMode;									// If the mode of Editfensters (0 = 1 = Edit ComboBox ComboBox 2 = fix)
	unsigned	uEditItem;									// If the entry is being edited
	unsigned	uEditSub;									// If the column is being edited
	unsigned	uOldXPage;									// Old values ??for X-scroll bar
	unsigned	uOldXCount;									// *
	unsigned	uOldYPage;									// Old values ??for Y-scroll bar
	unsigned	uOldYCount;									// *
	unsigned	uTrippleB;									// Prepare the "..." Strings for the fat fonts
	unsigned	uTrippleN;									// Prepare the "..." Strings for normal fonts
	unsigned	uTrackedItem;								// If the entry is to be emphasized
	unsigned	uTrackedSub;								// If the column of the entry is to be emphasized
	unsigned	uInsertMark;								// If the item with the cursor
	unsigned	uMarkedCols;								// Number of selected columns
	unsigned	uDragFlags;									// Which mouse buttons are on
	unsigned	uDragItem;									// Entry for drag operation
	unsigned	uDragSub;									// The entry for drag operation
	unsigned	uLastSel;									// Letzte Textauswahl beim Editieren
	unsigned	uLastMove;									// Letzte Cursorposition bei WM_MOUSEMOVE
	unsigned 	uButtonPos;									// Wo wurde eine Maustaste wurde zuletzt gedrückt 
	unsigned 	uButtonLast;								// Wann wurde eine Maustaste wurde zuletzt gedrückt
	unsigned	uToolTipSize;								// Textspeichergröße für Tooltip
	LPTSTR		pToolTipText;								// Textspeicher für Tooltip
	TCHAR		cTempText1   [260];							// First text buffer for callbacks
	TCHAR		cTempText2   [260];							// Text buffer Zeiter for callbacks
	int			aColumnXpos  [MAX_COLUMNS+2];				// Array with the positions of columns
	ColumnData	aColumn      [MAX_COLUMNS];					// The data columns
	char		cColorChanged[MAX_COLORS ];					// What color were changed
	char   	    cColumnStart;								// If the Autoeditiren with a WM_CHAR input starts
	char 		cFixedHeight;								// If a fixed amount set
	char		cLockChanges;								// Lock the window changes
	char		cHasRootRow;								// Will be set if a root column is inserted
	char		cKeyIgnore;									// The next button is not used for viewfinder
	char		cClickFlag;									// Flag for LButton-DOWN at Multiselct
	char		cClickEdit;									// Merker für LBUTTON-DOWN bei Edit-Click
	char		cIsEnabled;									// If the window is released
	char		cHasFocus;									// If the window the focus
	char		cReSelect;									// If the selection selects new
	char		cGlyphOk;									// Die Schaltfäche über Themen zeichnen
	char		cEditCb;									// Does the Edit window, invoke a callback
	char		cButtonFlag;								// Which mouse button was last pressed

	// IMI Hydronic Engineering variables.
	// Remark: these variables are internally set when calling 'TASetVAlternateColor' method.
	bool		fVAlternateMode;							// If 'true', background color is depending of item state.
	COLORREF	uCheckedBkgColor;							// Background color when item is checked.
	COLORREF	uUnCheckedBkgColor;						// Background color when item is unchecked.
	bool		fTADrawAllLine;								// - 'true' if we draw all the line with 'uCheckedBkgColor' color (included collapse/expand button, 
															//   checkbox, icon, text and spaces after).
															// - 'false' if we draw with 'uCheckedBkgColor' until collapse/expand button and with 'uUnCheckedBkgColor' 
															//   for all the rest of the line.
	unsigned	uTAWiderItem;
	}TreeListData;

typedef		TVSORTEX			   *LPTVSORTEX;
typedef		TVSORTEX			   *LPTV_SORTEX;
typedef		TVSORTEX				TV_SORTEX;


#define 	TVCOLUMN				LV_COLUMN
#define 	TV_COLUMN				LV_COLUMN
#define 	TV_FIND					TVFIND
#define 	TV_NOIMAGE				(-2)
#define 	TV_NOCOLOR				0xFFFFFFFF
#define 	TVLE_DONOTIFY			0xF5A5A500
#define 	TVIF_TEXTPTR			0x80000000
#define 	TVIF_TOOLTIPTIME		0x40000000
#define		TVIF_TEXTCHANGED		0x20000000
#define		TVIF_RETURNEXIT			0x10000000
#define 	TVIF_CASE				0x08000000
#define 	TVIF_NEXT				0x04000000
#define 	TVIF_CHILD				0x02000000
#define 	TVIF_CANCELED			0x01000000
#define 	TVIF_ONLYFOCUS			0x00800000
#define		TVIF_SUBITEM	        0x8000
#define 	TVIF_SUBNUMBER			0x4000
#define 	TVIS_UNTERLINE			0x0001
#define 	TVSIL_CHECK				0x0003
#define 	TVSIL_SUBIMAGES			0x0004
#define		TVN_COLUMNCLICK			HDN_ITEMCLICK
#define 	TVN_COLUMNDBLCLICK		HDN_ITEMDBLCLICK
#define 	TVI_SORTEX				((HTREEITEM) 0xFFFF0007)
#define 	TVI_BEFORE				((HTREEITEM) 0xFFFF0006)
#define 	TVI_AFTER				((HTREEITEM) 0xFFFF0005)
#define		TVI_ROW(n)		        ((HTREEITEM)(0xFFE00000+(n)))
#ifndef		VK_DBLCLK
#define 	VK_DBLCLK				0x10000				// Edit mit Doppelklick
#endif 	
#ifndef		VK_ICONCLK
#define 	VK_ICONCLK				0x10001				// Edit mit Klick auf Icon
#endif 	
#ifndef		VK_EDITCLK
#define 	VK_EDITCLK				0x10002				// Edit mit Klick auf augewähltes Element
#endif 	

// Constants for colors (TVM_SETBKCOLOR)
#define 	TVC_BK					0					// Background color     
#define 	TVC_ODD					1					// Alternate color (see TVS_EX_ALTERNATECOLOR)
#define 	TVC_EVEN				2					// Alternate color wheel (see TVS_EX_ALTERNATECOLOR)
#define 	TVC_FRAME				3					// Color for separators (see TVS_EX_ITEMLINES)
#define 	TVC_TEXT				4					// Text color
#define 	TVC_LINE				5					// Interior color of the buttons
#define 	TVC_BOX					6					// Exterior color of the buttons
#define 	TVC_TRACK				7					// Text color for getrakte (Farbe für getrakt Texte)
#define 	TVC_MARK				8					// Marked line
#define 	TVC_MARKODD				8					// Marked line (odd)
#define 	TVC_MARKEVEN			9					// Marked line (straight)
#define 	TVC_INSERT				10					// Color of the cursor
#define 	TVC_BOXBG				11					// Basic color of the buttons
#define 	TVC_COLBK				12					// Background color if column marked
#define 	TVC_COLODD				13					// Alternate color if column marked
#define 	TVC_COLEVEN				14					// Alternate color just when column marked
#define 	TVC_GRAYED				15					// Background color when obstacles blocked


// Konstanten für GetNextItem (TVM_GETNEXTITEM)
#define 	TVGN_DROPHILITESUB		0x000C				// Ausgewählte Spalte holen
#define 	TVGN_CARETSUB			0x000D				// Drophighilite Spalte holen
#ifndef		TVGN_NEXTSELECTED
#define 	TVGN_NEXTSELECTED		0x000E				// Nächster ausgewählter Eintrag
#endif
#define 	TVGN_FOCUS				0x000F				// Eintrag der den Focus hat
#define 	TVGN_FOCUSSUB			0x0010				// Spalte  die den Focus hat
#define 	TVGN_NEXTSELCHILD		0x0011				// Nächster ausgewählter Kindeintrag
#define 	TVGN_LASTCHILD			0x0012				// Letzter Kindeintrag
#define 	TVGN_NEXTITEM			0x0013				// Für Enumerierung der Einträge



// Konstanten für InsertColumn (mask)
#define 	TVCF_FMT				LVCF_FMT			// Die Textausrichting einstellen
#define 	TVCF_IMAGE				LVCF_IMAGE			// Ein Icon der Spalte zu ortenen
#define 	TVCF_TEXT				LVCF_TEXT			// Den Text der Spalte setzen
#define 	TVCF_WIDTH				LVCF_WIDTH			// Fixe Breite festlegen
#define 	TVCF_VWIDTH				LVCF_SUBITEM		// Variable Breite festlegen
#define 	TVCF_MIN				LVCF_ORDER			// Minimale Breite festlegen
#define 	TVCF_MARK				0x80000000			// Soll die Spalte markiert werden
#define 	TVCF_FIXED				0x40000000			// Darf die Spaltenbreite verändert werden


// Konstanten für ColumnAutoEdit
#define 	TVAE_NONE				(0<<TVAE_MODEPOS)	// Kein atomatisches Editieren
#define 	TVAE_EDIT				(1<<TVAE_MODEPOS)	// Atomatisches Editieren mit Edit-Fenster
#define 	TVAE_COMBO				(2<<TVAE_MODEPOS)	// Atomatisches Editieren mit ComboBox
#define 	TVAE_CBLIST				(3<<TVAE_MODEPOS)	// Atomatisches Editieren mit ComboListBox
#define 	TVAE_STEP				(4<<TVAE_MODEPOS)	// Einzelnes Weiterschalten mit Enter
#define 	TVAE_STEPED				(5<<TVAE_MODEPOS)	// Einzelnes Weiterschalten mit Enter
#define 	TVAE_CHECK				(6<<TVAE_MODEPOS)	// Atomatisches Editieren mit CheckBox
#define 	TVAE_CHECKED			(7<<TVAE_MODEPOS)	// Atomatisches Editieren mit CheckBox mit Edit
#define 	TVAE_NEXTLINE			0x0001				// Nach Enter in die nächste Zeile
#define 	TVAE_DBLCLICK			0x0002				// Editieren mit Doppelklick
#define 	TVAE_FULLWIDTH			0x0004				// Combobox über die volle Breite einblenden
#define 	TVAE_PTRLIST			0x0008				// Die Datenliste wird als Zeigerliste übergeben
#define 	TVAE_ONLYRETURN			0x0010				// Das Editeren kann nur mit Return begonnen werden
#define 	TVAE_STATEENABLE		0x0020				// Das Editeren kann mit dem 0x8000 State-Bit gesperrt werden
#define 	TVAE_ICONCLICK			0x0040				// Editieren mit einem Einfachlick auf das Icon
#define 	TVAE_DROPDOWN			0x0080				// Die DropDownListe der ComboBox aufklappen 
#define 	TVAE_COL(c)				(((c)&0x3F)<<11)	// Ist die Spalte fürs automatische Editieren
#define 	TVAE_CHAR(c)			(((c)&0xFF)<<17)	// Ist das Trennzeichen für die Datenliste
#define 	TVAE_COUNT(c)			(((c)&0x7F)<<25)	// Bestimmt die Anzahl der Einträge in der Datenliste (0=auto)
#define 	TVAE_MODEMASK			(7<<TVAE_MODEPOS)	// Maske für die Modusbits
#define 	TVAE_MODEPOS			0x0008				// Maske für die Modusbits
#define 	TVIS_DISABLEBIT			0x8000				// Ist das State-Bit für's sperren der Auto-Edit-Option


// Konstanten für HitTest (flags)
#define 	TVHT_SUBTOCOL(u)		(((unsigned)u)>>24)	// Umwandeln von Spaltemasken zu Spaltenummern
#define 	TVHT_SUBMASK			0xFF000000			// Maske in der die Spalte gespeichert wird
#define 	TVHT_ONRIGHTSPACE		0x00800000			// Auf rechtem Rand nach den Exträgen
#define 	TVHT_ONSUBLABEL			0x00400000			// Koordinate ist auf dem Text eines Extraeintrages
#define 	TVHT_ONSUBICON			0x00200000			// Koordinate ist auf einem Extraeintrag
#define 	TVHT_ONSUBRIGHT			0x00100000			// Koordinate ist auf einem Extraeintrag rechts vom Text
#define 	TVHT_ONSUBITEM			(TVHT_ONSUBICON|TVHT_ONSUBLABEL)


// Konstanten für GetItemRect (TVM_GETITEMRECT)
#define 	TVIR_COLTOSUB(u)		((u)<<24)			// Splate angeben
#define 	TVIR_GETCOLUMN			0x00000080			// Nur Splate abfragen
#define 	TVIR_TEXT				0x00000001			// Nur den Textbereich abfragen


// Konstanten für SelectChilds (TVM_SELECTCHILDS)
#define 	TVIS_WITHCHILDS			0x00000001			// Auch Kinder auswählen
#define 	TVIS_DESELECT			0x00000002			// Einträge abwählen

// Konstanten für Optionen (TVM_GETSETOPTION)
#define 	TVOP_AUTOEXPANDOFF		0x00000001			// Icon Offset für TVS_EX_AUTOEXPANDICON
#define 	TVOP_WRITEOPTION		0x80000000			// Die Option auch schreiben


// Konstanten für EditLabel (LVM_EDITLABEL)
#define 	TVIR_EDITCOMBOCHAR(n)	(((n)&0xFF)<<8)		// Trennzeichen der Combobox-Einträge (nur für Notify-Nachricht)
#define 	TVIR_EDITCOMBODEL		0x00000080			// Löscht den Puffer für die Einträge (nur für Notify-Nachricht)
#define 	TVIR_EDITCOMBODOWN		0x10000000			// Die Combobox aufklappen (nur für Notify-Nachricht)
#define 	TVIR_EDITCOMBOBOX		0x20000000			// Statt dem Edit-Fenster eine ComboBox einblenden
#define 	TVIR_EDITCOMBOLIST		0x40000000			// Statt dem Edit-Fenster eine ComboBox nur mit Listenauswahl einblenden
#define 	TVIR_EDITFULL			0x80000000			// Das Edit-Fenster über die volle Breite einblenden
#define 	TVIR_EDITTEXT			0x00000001			// Das Edit-Fenster über den Text einblenden (nur für Notify-Nachricht)
#define 	TVIR_EDITCOL(u)			((u)&0xFF)			// Splate angeben
#define 	TVIR_SELALL				0x00000000			// Alles wählen
#define 	TVIR_SELAREA(a,b)		((0x0C0000|(a&0x1FF)|((b&0x1FF)<<9))<<8)	// Textbereich wählen
#define 	TVIR_SETCURSOR(a)		((0x080000|(a&0x3FFFF))<<8)					// Cursor auf Textstelle
#define 	TVIR_SETAT(a)			((0x040000|(a&0x3FFFF))<<8)					// Cursor auf Pixel-Offset


// Konstanten für InsertColumn (mask)
#define 	TVCFMT_BITMAP_ON_RIGHT	LVCFMT_BITMAP_ON_RIGHT
#define 	TVCFMT_COL_HAS_IMAGES	LVCFMT_COL_HAS_IMAGES
#define 	TVCFMT_CENTER			LVCFMT_CENTER
#define 	TVCFMT_IMAGE			LVCFMT_IMAGE
#define 	TVCFMT_LEFT				LVCFMT_LEFT
#define 	TVCFMT_RIGHT			LVCFMT_RIGHT
#define 	TVCFMT_FIXED			0x20000000			// Maske für Spalten fixieren
#define 	TVCFMT_MARK				0x10000000			// Maske für Spalten markieren


// Konstanten für uStyleEx
#define 	TVS_EX_SINGLECHECKBOX	0x00002000			// Only simple selection from checkboxes
#define 	TVS_EX_STEPOUT			0x00004000			// Edit with cursor left and checkboxes
#define 	TVS_EX_BITCHECKBOX		0x00008000		    // Change checkboxes in only the lowest bit
#define 	TVS_EX_ITEMLINES		0x00010000		    // To draw a border all around the item (included buttons, icons and text)...
#define 	TVS_EX_ALTERNATECOLOR	0x00020000			// The color display alternately
#define 	TVS_EX_SUBSELECT		0x00040000		    // Sub-items separately select
#define 	TVS_EX_FULLROWMARK		0x00080000		    // Highlight the entire row if selected
#define 	TVS_EX_TOOLTIPNOTIFY	0x00100000		    // Notify-Specific query for tooltips
#define 	TVS_EX_AUTOEXPANDICON	0x00200000		    // Automatically use the next icon in Expandet
#define 	TVS_EX_NOCHARSELCET		0x00400000		    // No selection of a text input
#define 	TVS_EX_NOCOLUMNRESIZE	0x00800000		    // The user may not change the column width
#define 	TVS_EX_HIDEHEADERS		0x01000000		    // Hide the header bar
#define 	TVS_EX_GRAYEDDISABLE	0x02000000		    // The control Gray characterized if it is locked
#define 	TVS_EX_FULLROWITEMS		0x04000000		    // Entries on the whole line of characters
#define 	TVS_EX_FIXEDCOLSIZE		0x08000000		    // Total width of all Splaten leave immediately
#define 	TVS_EX_HOMEENDSELECT	0x10000000			// Mit Ctrl+Home/End das erste und letze Element wählen
#define 	TVS_EX_SHAREIMAGELISTS	0x20000000			// Image-Listen nicht löschen bei Fensterzerstörung
#define 	TVS_EX_EDITCLICK		0x40000000			// Mit einem Klick das gewählte Item berabreiten
#define 	TVS_EX_NOCURSORSET		0x80000000			// Bei VK_EDITCLK immer den gesammten Text wählen
#ifndef		TVS_EX_MULTISELECT
#define 	TVS_EX_MULTISELECT		0x00000002			// Eine Mehrfachauswahl erlauben
#endif
#ifndef		TVS_EX_AUTOHSCROLL
#define		TVS_EX_AUTOHSCROLL      0x00000020			// Horizontale Scrollbar ausblenden
#endif


// Constants for Notify messages.
#define		TVN_ITEMTOOLTIP			(TVN_FIRST-32)		// Notify message for tooltip window
#define     TVN_CBSTATECHANGED      (TVN_FIRST-33)		// Notify message for changed checkbox
#define		TVN_STEPSTATECHANGED	(TVN_FIRST-34)		// Notify message for changed state AutoEdit
#define		TVN_STARTEDIT			(TVN_FIRST-35)		// Notify message fields to edit


// Constants for new messages.
#define 	TVM_GETHEADER			(TV_FIRST+96-1)
#define 	TVM_GETCOLUMNCOUNT		(TV_FIRST+96-2)
#define 	TVM_DELETECOLUMN		(TV_FIRST+96-3)
#define 	TVM_INSERTCOLUMN		(TV_FIRST+96-4)
#define 	TVM_SELECTSUBITEM		(TV_FIRST+96-5)
#define 	TVM_SELECTDROP			(TV_FIRST+96-6)
#define 	TVM_SETITEMBKCOLOR		(TV_FIRST+96-7)
#define 	TVM_GETITEMBKCOLOR		(TV_FIRST+96-8)
#define 	TVM_SETITEMTEXTCOLOR	(TV_FIRST+96-9)
#define 	TVM_GETITEMTEXTCOLOR	(TV_FIRST+96-10)
#define 	TVM_GETITEMOFROW		(TV_FIRST+96-11)
#define 	TVM_GETROWCOUNT			(TV_FIRST+96-12)
#define 	TVM_GETROWOFITEM		(TV_FIRST+96-13)
#define 	TVM_SETCOLUMN			(TV_FIRST+96-14)
#define 	TVM_GETCOLUMN			(TV_FIRST+96-15)
#define 	TVM_SETCOLUMNWIDTH		(TV_FIRST+96-16)
#define 	TVM_GETCOLUMNWIDTH		(TV_FIRST+96-17)
#define 	TVM_SETUSERDATASIZE		(TV_FIRST+96-18)
#define 	TVM_GETUSERDATASIZE		(TV_FIRST+96-19)
#define 	TVM_GETUSERDATA			(TV_FIRST+96-20)
#define 	TVM_SORTCHILDRENEX		(TV_FIRST+96-21)
#define 	TVM_COLUMNAUTOEDIT		(TV_FIRST+96-22)
#define 	TVM_COLUMNAUTOICON		(TV_FIRST+96-23)
#define 	TVM_GETCOUNTPERPAGE		(TV_FIRST+96-24)
#define 	TVM_FINDITEM			(TV_FIRST+96-25)
#define 	TVM_SELECTCHILDS		(TV_FIRST+96-26)
#define 	TVM_GETSETOPTION		(TV_FIRST+96-27)
#define 	TVM_ISITEMVISIBLE		(TV_FIRST+96-28)
#define 	TVM_SETFOCUSITEM		(TV_FIRST+96-29)

// IMI Hydronic Engineering: Only one more message is available for use (TV_FIRST+96-31 will overwrite other windows defines).
// Thus we use only one message for all new implemented methods. 
// WPARAM will contain code for method and.
// LPARAM will contain a pointer on a structure with all needed data.
#define		TVM_TAHYDRONICSMETHODS	(TV_FIRST+96-30)
#define		TAMETHODCODE_GETTREELISTDATA		0x01				// Allow to retrieve pointer on 'TreeListData'.
#define		TAMETHODCODE_ENSUREVISIBLE			0x02				// Horizontal scroll if necessary to be sure that an item is completely shown.
#define		TAMETHODCODE_GETITEMRECT			0x03				// Method similar to MFC GetItemRect but fills rectangle even if item is not visible.
#define		TAMETHODCODE_SETVALTERNATECOLOR		0x04				// Allow to draw tree background colors in regards to current item state.

// Structure for 'TAEnsureVisible' method.
typedef struct
{
	unsigned uItem;
	bool fMoveLeft;		// Set to 'true' if part of the item is at the left of the client area border, otherwise it's the right border.
}TAEnsureVisibleParam;

// Structure for 'TAGetItemRect' method.
typedef struct
{
	unsigned uItem;
	CRect Rect;
	bool fTextOnly;
}TAGetItemRectParam;

// Structure for 'TASetVAlternateColor' method.
typedef struct
{
	bool	 fVAlternateMode;		// 'true' if we must use special background painting for diversity factor.
	bool	 fDrawAllLine;			// - 'true' if we draw all the line with 'uCheckedBkgColor' color (included collapse/expand button, checkbox, icon, text and 
									//   spaces after).
									// - 'false' if we draw with 'uCheckedBkgColor' until collapse/expand button and with 'uUnCheckedBkgColor' for all the rest 
									//   of the line.
	COLORREF uCheckedBkgColor;
	COLORREF uUnCheckedBkgColor;
}TASetVAlternateColorParam;

#ifndef		TVM_GETITEMSTATE
#define 	TVM_GETITEMSTATE		(TV_FIRST+39)
#endif
#ifndef		TVM_GETEXTENDEDSTYLE
#define 	TVM_GETEXTENDEDSTYLE	(TV_FIRST+45)
#endif
#ifndef		TVM_SETEXTENDEDSTYLE
#define 	TVM_SETEXTENDEDSTYLE	(TV_FIRST+44)
#endif
#ifndef		TVM_GETLINECOLOR
#define 	TVM_GETLINECOLOR		(TV_FIRST+41)
#endif
#ifndef		TVM_SETLINECOLOR
#define 	TVM_SETLINECOLOR		(TV_FIRST+40)
#endif


#ifndef		TVNRET_DEFAULT
#define		TVNRET_DEFAULT			0
#endif
#ifndef		TVNRET_SKIPOLD
#define		TVNRET_SKIPOLD			1
#endif
#ifndef		TVNRET_SKIPNEW
#define		TVNRET_SKIPNEW			2
#endif

#define 	TreeList_DeleteChildItems(h,i)				((BOOL      )::SendMessage(h,TVM_DELETEITEM,0x88,(LPARAM)i))
#define 	TreeList_DeleteAllItems(h)					((BOOL      )::SendMessage(h,TVM_DELETEITEM,0,(LPARAM)TVI_ROOT))
#define		TreeList_DeleteItem(h,i)					((BOOL      )::SendMessage(h,TVM_DELETEITEM,0,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_Expand(h,i,c)						((BOOL      )::SendMessage(h,TVM_EXPAND,(WPARAM)(c),(LPARAM)(HTREEITEM)(i)))
#define		TreeList_GetHeader(h)						((HWND      )::SendMessage(h,TVM_GETHEADER,0,0))
#define		TreeList_DeleteColumn(h,i)     				((BOOL      )::SendMessage(h,TVM_DELETECOLUMN,(WPARAM)(int)(i),0))
#define		TreeList_InsertColumn(h,i,p)				((INT       )::SendMessage(h,TVM_INSERTCOLUMN,(WPARAM)(int)(i),(LPARAM)(const TV_COLUMN*)(p)))
#define 	TreeList_GetColumnCount(h)					((INT       )::SendMessage(h,TVM_GETCOLUMNCOUNT,0,0))
#define 	TreeList_HitTest(h,p)						((HTREEITEM )::SendMessage(h,TVM_HITTEST,0,(LPARAM)(LPTV_HITTESTINFO)(p)))
#define 	TreeList_GetItemOfRow(h,r)					((HTREEITEM )::SendMessage(h,TVM_GETITEMOFROW,0,r))
#define 	TreeList_GetRowOfItem(h,i)					((INT       )::SendMessage(h,TVM_GETROWOFITEM,0,(LPARAM)(i)))
#define 	TreeList_GetRowCount(h)						((INT       )::SendMessage(h,TVM_GETROWCOUNT ,0,0))
#define 	TreeList_GetCountPerPage(h)					((INT       )::SendMessage(h,TVM_GETCOUNTPERPAGE ,0,0))
#define		TreeList_GetExtendedStyle(h)				((DWORD     )::SendMessage(h,TVM_GETEXTENDEDSTYLE,0,0))
#define		TreeList_SetExtendedStyle(h,d)	  			((DWORD     )::SendMessage(h,TVM_SETEXTENDEDSTYLE,0,d))
#define 	TreeList_SetExtendedStyleEx(h,d,m)			((DWORD     )::SendMessage(h,TVM_SETEXTENDEDSTYLE,m,d))
#define		TreeList_GetColor(h,i)						((COLORREF  )::SendMessage(h,TVM_GETBKCOLOR,(WPARAM)(i),0))
#define		TreeList_SetColor(h,i,c) 					((COLORREF  )::SendMessage(h,TVM_SETBKCOLOR,(WPARAM)(i),c))
#define		TreeList_GetItemBkColor(h,i,s)				((COLORREF  )::SendMessage(h,TVM_GETITEMBKCOLOR,(WPARAM)(i),s))
#define		TreeList_SetItemBkColor(h,i,s,c) 			((COLORREF  )::SendMessage(h,TVM_SETITEMBKCOLOR,((UINT)(i))|((s)<<24),c))
#define		TreeList_GetItemTextColor(h,i,s)			((COLORREF  )::SendMessage(h,TVM_GETITEMTEXTCOLOR,(WPARAM)(i),s))
#define		TreeList_SetItemTextColor(h,i,s,c) 			((COLORREF  )::SendMessage(h,TVM_SETITEMTEXTCOLOR,((UINT)(i))|((s)<<24),c))
#define 	TreeList_IsItemVisible(h,i,s)				((INT       )::SendMessage(h,TVM_ISITEMVISIBLE,s,(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_EnsureVisible(h,i)					((BOOL      )::SendMessage(h,TVM_ENSUREVISIBLE,0,(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_EnsureVisibleEx(h,i,s)				((BOOL      )::SendMessage(h,TVM_ENSUREVISIBLE,s,(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_SelectDropTargetEx(h,i,s)			((BOOL      )::SendMessage(h,TVM_SELECTDROP,(WPARAM)(s),(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_SelectSubItem(h,i,s)				((BOOL      )::SendMessage(h,TVM_SELECTSUBITEM,(WPARAM)(s),(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_SelectChilds(h,i,s)				((BOOL      )::SendMessage(h,TVM_SELECTCHILDS,(WPARAM)(s),(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_Select(h,i,c)						((BOOL      )::SendMessage(h,TVM_SELECTITEM,(WPARAM)(c),(LPARAM)(HTREEITEM)(i)))
#define		TreeList_EditLabel(h,i,s)				    ((HWND      )::SendMessage(h,TVM_EDITLABEL,s,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_StartEdit(h,i,s)				    ((BOOL      )::SendMessage(h,TVM_EDITLABEL,TVIR_EDITCOL(s)|TVLE_DONOTIFY,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_EndEditLabelNow(h,c)				((BOOL      )::SendMessage(h,TVM_ENDEDITLABELNOW,c,0))
#define		TreeList_GetItem(h,p)						((BOOL      )::SendMessage(h,TVM_GETITEM,0,(LPARAM)(TV_ITEM*)(p)))
#define		TreeList_GetCount()							((BOOL      )::SendMessage(h,TVM_GETCOUNT,0,0))
#define		TreeList_GetEditControl(h)					((HWND      )::SendMessage(h,TVM_GETEDITCONTROL,0,0))
#define		TreeList_GetImageList(h,i)					((HIMAGELIST)::SendMessage(h,TVM_GETIMAGELIST,i,0))
#define		TreeList_GetUserData(h,i)					((LPVOID    )::SendMessage(h,TVM_GETUSERDATA,0,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_GetUserDataSize(h)					((INT       )::SendMessage(h,TVM_GETUSERDATASIZE,0,0))
#define		TreeList_SetUserDataSize(h,s)				((INT       )::SendMessage(h,TVM_SETUSERDATASIZE,0,s))
#define		TreeList_GetIndent							((UINT      )::SendMessage(h,TVM_GETINDENT,0,0))
#define		TreeList_GetVisibleCount					((UINT      )::SendMessage(h,TVM_GETVISIBLECOUNT,0,0))
#define		TreeList_InsertItem(h,p)					((HTREEITEM )::SendMessage(h,TVM_INSERTITEM,0,(LPARAM)(LPTV_INSERTSTRUCT)(p)))
#define		TreeList_FindItem(h,p,f)					((HTREEITEM )::SendMessage(h,TVM_FINDITEM ,(WPARAM)p,(LPARAM)f))
#define		TreeList_CreateDragImage(h,i)				((HIMAGELIST)::SendMessage(h,TVM_CREATEDRAGIMAGE, 0, (LPARAM)(HTREEITEM)(i)))
#define		TreeList_CreateDragImageEx(h,i,s)			((HIMAGELIST)::SendMessage(h,TVM_CREATEDRAGIMAGE, s, (LPARAM)(HTREEITEM)(i)))
#define		TreeList_SetImageList(h,l,i)				((HIMAGELIST)::SendMessage(h,TVM_SETIMAGELIST,i,(LPARAM)(UINT)(HIMAGELIST)(l)))
#define		TreeList_SetIndent(h,i)					    ((BOOL      )::SendMessage(h,TVM_SETINDENT,(WPARAM)(i),0))
#define		TreeList_SetItem(h,p)					    ((BOOL      )::SendMessage(h,TVM_SETITEM,0,(LPARAM)(const TV_ITEM*)(p)))
#define		TreeList_SortChildren(h,i,r)				((BOOL      )::SendMessage(h,TVM_SORTCHILDREN  ,(WPARAM)r,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_SortChildrenCB(h,p,r)				((BOOL      )::SendMessage(h,TVM_SORTCHILDRENCB,(WPARAM)r,(LPARAM)(LPTV_SORTCB)(p)))
#define		TreeList_SortChildrenEX(h,p,r)				((BOOL      )::SendMessage(h,TVM_SORTCHILDRENEX,(WPARAM)r,(LPARAM)(LPTV_SORTEX)(p)))
#define		TreeList_SetColumn(h,i,p)					((BOOL      )::SendMessage(h,TVM_SETCOLUMN,i,(LPARAM)(const TV_COLUMN*)(p)))
#define		TreeList_GetColumn(h,i,p)					((BOOL      )::SendMessage(h,TVM_GETCOLUMN,i,(LPARAM)(TV_COLUMN*)(p)))
#define		TreeList_SetColumnWidth(h,i,w)				((BOOL      )::SendMessage(h,TVM_SETCOLUMNWIDTH,i,w))
#define		TreeList_GetColumnWidth(h,i)				((INT       )::SendMessage(h,TVM_GETCOLUMNWIDTH,i,0))
#define 	TreeList_SetColumnAutoEdit(h,i,f,p)			((BOOL      )::SendMessage(h,TVM_COLUMNAUTOEDIT,(WPARAM)((f)&~TVAE_COL(-1))|TVAE_COL(i),(LPARAM)(p)))
#define 	TreeList_SetColumnAutoIcon(h,i,n)			((BOOL      )::SendMessage(h,TVM_COLUMNAUTOICON,i,n))
#define 	TreeList_SetFocusItem(h,i,c)				((BOOL      )::SendMessage(h,TVM_SETFOCUSITEM,c,(LPARAM)(i)))
#define 	TreeList_SetOption(h,i,o)					((INT       )::SendMessage(h,TVM_GETSETOPTION,(i)|TVOP_WRITEOPTION,(LPARAM)(o)))
#define 	TreeList_GetOption(h,i)						((INT       )::SendMessage(h,TVM_GETSETOPTION,i,0))
#define		TreeList_GetStyle(h)						((DWORD     )::GetWindowLong(h,GWL_STYLE))
#define		TreeList_SetStyle(h,d)	  					((DWORD     )::SetWindowLong(h,GWL_STYLE,d))
#define 	TreeList_SetStyleEx(h,d,m)					((DWORD     )::SetWindowLong(h,GWL_STYLE,((d)&(m))|(::GetWindowLong(h,GWL_STYLE)&~(m))))
#define		TreeList_GetItemRect(h,i,s,p,c)			    (*(HTREEITEM*)p =(i),(BOOL)::SendMessage(h,TVM_GETITEMRECT,(WPARAM)((c)|(TVIR_COLTOSUB(s))),(LPARAM)(RECT*)(p)))

#define		TreeList_SelectItem(h,i)					TreeList_Select(h,i,TVGN_CARET)
#define 	TreeList_SelectDropTarget(h,i)				TreeList_Select(h,i,TVGN_DROPHILITE)
#define 	TreeList_SelectSetFirstVisible(h,i)			TreeList_Select(h,i,TVGN_FIRSTVISIBLE)

#define 	TreeList_GetNextItem(h,i,c)					TreeView_GetNextItem(h, i,     c)
#define		TreeList_GetChild(h,i)						TreeView_GetNextItem(h, i,     TVGN_CHILD)
#define 	TreeList_GetParent(h, i)         			TreeView_GetNextItem(h, i,     TVGN_PARENT)
#define 	TreeList_GetNextSibling(h,i)    			TreeView_GetNextItem(h, i,     TVGN_NEXT)
#define 	TreeList_GetPrevSibling(h,i)    			TreeView_GetNextItem(h, i,     TVGN_PREVIOUS)
#define 	TreeList_GetNextSelected(h,i)			    TreeView_GetNextItem(h, i,     TVGN_NEXTSELECTED)
#define		TreeList_GetNextSelectedChild(h,i)			TreeView_GetNextItem(h, i,	   TVGN_NEXTSELCHILD)
#define 	TreeList_GetNextVisible(h,i)    			TreeView_GetNextItem(h, i,     TVGN_NEXTVISIBLE)
#define 	TreeList_GetPrevVisible(h,i)    			TreeView_GetNextItem(h, i,     TVGN_PREVIOUSVISIBLE)
#define 	TreeList_GetLastChild(h,i)				    TreeView_GetNextItem(h, i,     TVGN_LASTCHILD)
#define 	TreeList_GetSelection(h)					TreeView_GetNextItem(h, NULL,  TVGN_CARET)
#define 	TreeList_GetDropHilight(h)					TreeView_GetNextItem(h, NULL,  TVGN_DROPHILITE)
#define 	TreeList_GetFirstVisible(h)				    TreeView_GetNextItem(h, NULL,  TVGN_FIRSTVISIBLE)
#define		TreeList_GetLastVisible(h)					TreeView_GetNextItem(h, NULL,  TVGN_LASTVISIBLE)
#define 	TreeList_GetRoot(h)							TreeView_GetNextItem(h, NULL,  TVGN_ROOT)
#define		TreeList_GetFocus(h)						TreeView_GetNextItem(h, NULL,  TVGN_FOCUS)
#define		TreeList_GetFocusColumn(h)					((int)TreeView_GetNextItem(h, NULL,  TVGN_FOCUSSUB))
#define		TreeList_GetSelectionColumn(h)				((int)TreeView_GetNextItem(h, NULL,  TVGN_CARETSUB))
#define		TreeList_GetDropHilightColumn(h)			((int)TreeView_GetNextItem(h, NULL,  TVGN_DROPHILITESUB))

// IMI Hydronic Engineering: New method to get access to tree list internal variables.
#define		TATreeList_GetTreeListData( h )				( (TreeListData*)::SendMessage( h, TVM_TAHYDRONICSMETHODS, TAMETHODCODE_GETTREELISTDATA, 0 ) )

// IMI Hydronic Engineering: Method to allow horizontal scrolling if necessary.
#define		TATreeList_EnsureVisible( h, i )			( (BOOL)::SendMessage( h, TVM_TAHYDRONICSMETHODS, TAMETHODCODE_ENSUREVISIBLE, (LPARAM)i ) )

// IMI Hydronic Engineering: Method similar to MFC GetItemRect but fills rectangle even if item is not visible.
#define		TATreeList_GetItemRect( h, p )			    ( (BOOL)::SendMessage( h, TVM_TAHYDRONICSMETHODS, TAMETHODCODE_GETITEMRECT, (LPARAM)p ) )

// IMI Hydronic Engineering: Set to true if tree must display background colors in regards to item state.
#define		TATreeList_SetVAlternateColor( h, p )		( (BOOL)::SendMessage( h, TVM_TAHYDRONICSMETHODS, TAMETHODCODE_SETVALTERNATECOLOR, (LPARAM)p ) )

int			TreeListRegister(HINSTANCE hInstance=0);



#endif
