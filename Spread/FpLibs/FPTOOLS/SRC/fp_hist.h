/*-------------------------------------------------------------------------

FP_HIST.H  -  Strictly comments listing the changes to FPTOOLS library.         
                                                                       
HISTORY OF CHANGES:
-----------------------                                                
                                                                       
$00   02/08/99  SCP: Creation of this history log file.
$01   02/22/99  CTF: Changed picture paint and manager code to support
                     non 32 x 32 icons. 
                     Files: FPPICT.C, FPDIB.C, FPGRAPHC.C, FPTOOLS.H
$02   02/22/99  CTF: Added function fpDrawTransparentBitmapEx to support 
                     Height and Width parameters. 
                     Files: FPGRAPHC.C, FPTOOLS.H
$03	  02/26/99	SCL: Implenented enhancements request SEL8276 for ListPro
					 (search source for SEL8276 for the changes)
					 Added new flag DTX_ELLIPSES for fpDrawText and fpDrawTextEx
					 Use this new flag in the fuStyle member of the FPDRAWTEXT struct
					 to draw ellises at the end of text when it will not fit into the
					 rect.  Ignored if text is not left-aligned or single-line.
					 Files: FPTOOL40.C, FPTOOL46.C, FPTOOLS.H
$04	  03/26/99  KAM: Made change to implement ThreeDOutsideHighlightColor as a system
					 color.
					 Files: VBNCPNT.C, FPNCPNT.C
$05	  04/15/99	SCL: Fixed memory leaks in picture table.  Added call to fpDetachPicture
					 to code in fpPMx_AddItem in the case where picture is already in
					 the picture table.  Also moved code that decrements ref count of global
					 picture data manager in fpDestroyGlobalData so that the count is decreased
					 when the ref count on the particular picture table is decreased.  Finally,
					 removed redundant code in fpDetachPicture that was trying to release the
					 ref on the IPicture interface (calling GetPictureDispatch adds an extra
					 ref that the subsequent call to Release was freeing; this ref is released 
					 in the destructor for CPictureHolder anyway).
					 Files: FPPICT.C, FPTOOL15.C, FPWAPI3.CPP
$06   04/22/99  SCL: Fixed error I introduced in pict table code in DLL fptools.
					 Files: FPPICT.C
$07   03/02/00  SCP: Added use of "m_fIsLoadingWithHWnd" flag when setting
           properties in the control.  If this flag is set, then certain
           steps are skipped when loading the properties.
           Files: FPXPROP.CPP, FPATL.H, FPATLCTL.H          
$08   03/08/00  SCP: Override Invoke() function to map certain variant
           vartypes (VT_UI4, etc) to vartypes VB "knows better".
           Files: FPATLCTL.H
--------------------------------------------------------------------------*/
