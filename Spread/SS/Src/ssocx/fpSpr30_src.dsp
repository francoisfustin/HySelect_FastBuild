# Microsoft Developer Studio Project File - Name="fpSpr30" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fpSpr30 - Win32 ANSI Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fpSpr30_src.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fpSpr30_src.mak" CFG="fpSpr30 - Win32 ANSI Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fpSpr30 - Win32 ANSI Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fpSpr30 - Win32 ANSI Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fpSpr30 - Win32 ANSI Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\objd32"
# PROP BASE Intermediate_Dir ".\objd32"
# PROP BASE Classwizard_Name "ssocx.clw"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\objd32"
# PROP Intermediate_Dir ".\objd32"
# PROP Classwizard_Name "ssocx.clw"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "WIN32" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I ".\\" /I "..\sssrc\spread" /I "..\..\..\fplibs\fpdb\adb" /I "..\..\..\fplibs\msdasdk\include" /I "..\..\include" /I "..\..\..\fplibs\fpdb\src" /I "..\..\..\fplibs\fptools\src" /D "_DEBUG" /D "FP_ADB" /D "_MBCS" /D "VC50" /D "SS_V30" /D "_WINDOWS" /D "_AFXCTL" /D "WIN32" /D "SS_SPREADOCX" /D "SS_OCX" /D "SS_BOUNDCONTROL" /D "FP_OCX" /D "_WINDLL" /D "_AFXDLL" /D "BUGS" /D "VC60" /D "FP_OLEDRAG" /D "TT_V2" /D "ISOLATION_AWARE_ENABLED" /D "CS_OFF" /D "SS_NO_USE_SH" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /D "_WIN32" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /D "FP_ADB" /D "SS_V30" /D "_WIN32" /D "SS_SPREADOCX" /D "SS_OCX" /D "FP_OLEDRAG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /i "..\..\include" /d "_DEBUG" /d "_WIN32" /d "WIN32" /d "SS_SPREADOCX" /d "SS_OCX" /d "_AFXDLL" /d "SS_V30"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /i "..\..\include" /d "_DEBUG" /d "FP_ADB" /d "_WIN32" /d "WIN32" /d "SS_SPREADOCX" /d "SS_OCX" /d "_AFXDLL" /d "SS_V30" /d "CS_OFF"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o".\objd32\ssocx.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o".\objd32\ssocx.bsc"
# SUBTRACT BSC32 /Iu
LINK32=link.exe
# ADD BASE LINK32 mfcs42d.lib largeint.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /nodefaultlib:"LIBCMT" /nodefaultlib:"LIBC"
# SUBTRACT BASE LINK32 /pdb:none /incremental:no /nodefaultlib
# ADD LINK32 mfcs42d.lib fptools.lib xl32l30.lib jpeg32.lib Adb32.lib oledbd.lib adbctld.lib msdatsrc.lib largeint.lib user32.lib msimg32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /nodefaultlib:"LIBCMT" /nodefaultlib:"LIBC" /nodefaultlib:"LIBCD" /out:".\objd32\fpSpr30.OCX" /libpath:"..\..\..\fplibs\msdasdk\lib" /libpath:"..\..\..\fplibs\fptools\ocx\objd32\\" /libpath:"..\..\..\fplibs\fptools\jpeg\objd32" /libpath:"..\excel\objd32\\" /libpath:"..\..\..\FPLIBS\FPDB\Adb\objd32\\"
# SUBTRACT LINK32 /pdb:none /incremental:no /nodefaultlib
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\objd32
TargetPath=.\objd32\fpSpr30.OCX
InputPath=.\objd32\fpSpr30.OCX
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fpSpr30 - Win32 ANSI Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\obj32"
# PROP BASE Intermediate_Dir ".\obj32"
# PROP BASE Classwizard_Name "ssocx.clw"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\obj32"
# PROP Intermediate_Dir ".\obj32"
# PROP Classwizard_Name "ssocx.clw"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O1 /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O1 /Oy /I ".\\" /I "..\sssrc\spread" /I "..\..\..\fplibs\fpdb\adb" /I "..\..\..\fplibs\msdasdk\include" /I "..\..\include" /I "..\..\..\fplibs\fpdb\src" /I "..\..\..\fplibs\fptools\src" /D "NDEBUG" /D "FP_ADB" /D "_MBCS" /D "VC50" /D "SS_V30" /D "_WINDOWS" /D "_AFXCTL" /D "WIN32" /D "SS_SPREADOCX" /D "SS_OCX" /D "SS_BOUNDCONTROL" /D "FP_OCX" /D "_WINDLL" /D "_AFXDLL" /D "BUGS" /D "VC60" /D "FP_OLEDRAG" /D "TT_V2" /D "ISOLATION_AWARE_ENABLED" /D "CS_OFF" /D "SS_NO_USE_SH" /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "_WIN32" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "FP_ADB" /D "SS_V30" /D "_WIN32" /D "SS_SPREADOCX" /D "SS_OCX" /D "FP_OLEDRAG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /i "..\..\include" /d "NDEBUG" /d "_WIN32" /d "WIN32" /d "SS_SPREADOCX" /d "SS_OCX" /d "_AFXDLL" /d "SS_V30"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /i "..\..\include" /d "NDEBUG" /d "FP_ADB" /d "_WIN32" /d "WIN32" /d "SS_SPREADOCX" /d "SS_OCX" /d "_AFXDLL" /d "SS_V30" /d "CS_OFF"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o".\obj32\ssocx.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o".\obj32\ssocx.bsc"
# SUBTRACT BSC32 /Iu
LINK32=link.exe
# ADD BASE LINK32 mfcs42.lib largeint.lib ..\..\..\fplibs\fptools\ocx\obj32\fptools.lib uuid.lib ..\excel\ocx\release60\xl32l60.lib /nologo /subsystem:windows /dll /machine:IX86 /nodefaultlib:"LIBCMT" /nodefaultlib:"LIBC" /nodefaultlib:"uuid3"
# SUBTRACT BASE LINK32 /pdb:none /map /debug /nodefaultlib
# ADD LINK32 mfcs42.lib fptools.lib uuid.lib xl32l30.lib jpeg32.lib adbctl.lib oledb.lib msdatsrc.lib largeint.lib msimg32.lib user32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /nodefaultlib:"LIBCMT" /nodefaultlib:"LIBC" /nodefaultlib:"uuid3" /out:".\obj32\fpSpr30.OCX" /libpath:"..\..\..\fplibs\fptools\ocx\obj32\\" /libpath:"..\excel\obj32\\" /libpath:"..\..\..\fplibs\fptools\jpeg\obj32" /libpath:"..\..\..\fplibs\msdasdk\lib"
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\obj32
TargetPath=.\obj32\fpSpr30.OCX
InputPath=.\obj32\fpSpr30.OCX
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "fpSpr30 - Win32 ANSI Debug"
# Name "fpSpr30 - Win32 ANSI Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\SSSRC\ATOF.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_CALC.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_CELL.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_COL.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_CUST.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_ERR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_EVAL.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_EXPR.C
# End Source File
# Begin Source File

SOURCE=..\sssrc\calc\cal_fina.c
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_FUNC.C
# End Source File
# Begin Source File

SOURCE=..\sssrc\calc\cal_look.c
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_MATR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_MEM.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_NAME.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_OPER.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_ROW.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_TOK.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_VECT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\CALENDAR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\CARET.C
# End Source File
# Begin Source File

SOURCE=..\..\..\Fplibs\Fptools\Src\Cathelp.cpp
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\CHAR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CLASSES\CHECKBOX.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\DATEMGR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\DATETIME.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\DYNAMIC1.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\EDITDATE.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITFLD.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITFLOA\EDITFLOA.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITGEN\EDITGEN.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITINT\EDITINT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITPIC\EDITPIC.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITTIME\EDITTIME.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\FCVT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\FIELDMGR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITFLOA\FLOATMGR.C
# End Source File
# Begin Source File

SOURCE=..\..\..\FPLIBS\FPDB\ADB\FPDBMFC.CPP
# End Source File
# Begin Source File

SOURCE=..\..\..\Fplibs\Fptools\Src\Fpmemdll.c
# End Source File
# Begin Source File

SOURCE=.\fpSPR30.DEF
# End Source File
# Begin Source File

SOURCE=..\SSSRC\FSSTUB.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITGEN\GENMGR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\HILIGHT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITINT\INTMGR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\KEYBOARD.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\MEM.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\OUTPUT1.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\OUTPUT2.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\OUTPUT3.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\OVERMODE.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITPIC\PICMGR.C
# End Source File
# Begin Source File

SOURCE=..\Ssprvw\Pvctl.cpp
# End Source File
# Begin Source File

SOURCE=..\Ssprvw\Pvpaint.c
# End Source File
# Begin Source File

SOURCE=..\Ssprvw\Pvpinfo.c
# End Source File
# Begin Source File

SOURCE=..\Ssprvw\Pvprops.c
# End Source File
# Begin Source File

SOURCE=..\Ssprvw\Pvscroll.c
# End Source File
# Begin Source File

SOURCE=..\SSSRC\QSORTH.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CLASSES\SPINBTN.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SPREAD.C
# End Source File
# Begin Source File

SOURCE=..\ssprvwpp\SS30PP_i.c
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_ALLOC.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_BOOL.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_BORD.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_CALC.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_CELL.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_COL.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_COMBO.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_COPY.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_CURS.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_DATA.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_DOC.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_DRAG.C
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\SS_DRAW.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_EDIT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_EMODE.C
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_excel.c
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_FONT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_FORMU.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_FUNC.C
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\Ss_html.c
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_MAIN.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_MULTI.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_OVERF.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_PRINT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_ROW.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SAVE.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SAVE4.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SCBAR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SCROL.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SORT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SPICT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_TYPE.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_USER.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_VIRT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_WIN.C
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_xlload.c
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_xlparse.c
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_xml.c
# End Source File
# Begin Source File

SOURCE=.\SSOCX.CPP
# End Source File
# Begin Source File

SOURCE=.\SSOCX.ODL
# End Source File
# Begin Source File

SOURCE=.\SSOCX.RC
# End Source File
# Begin Source File

SOURCE=.\SSOCXCTL.CPP
# End Source File
# Begin Source File

SOURCE=.\SSOCXDB.CPP
# End Source File
# Begin Source File

SOURCE=.\SSOCXEVT.CPP
# End Source File
# Begin Source File

SOURCE=.\SSOCXMTH.CPP
# End Source File
# Begin Source File

SOURCE=.\Ssocxpbg.cpp
# End Source File
# Begin Source File

SOURCE=.\SSOCXPR2.CPP
# End Source File
# Begin Source File

SOURCE=.\SSOCXPRP.CPP
# End Source File
# Begin Source File

SOURCE=.\SSOCXVRB.CPP
# End Source File
# Begin Source File

SOURCE=..\Ssprvw\Ssprvw.c
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\STATIC.C
# End Source File
# Begin Source File

SOURCE=.\STDAFX.CPP
# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"
# End Source File
# Begin Source File

SOURCE=..\SSSRC\STR1.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\STR2.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CLASSES\SUPERBTN.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CLASSES\TBDIB.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\TEXT.C
# End Source File
# Begin Source File

SOURCE=..\..\..\FPLIBS\FPTOOLS\SRC\TEXTTIP.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITTIME\TIMEMGR.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\USERDATE.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITFLOA\USERFLOA.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\USERFUNC.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITINT\USERINT.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITTIME\USERTIME.C
# End Source File
# Begin Source File

SOURCE=..\VBSRC\VBBOUND.C
# End Source File
# Begin Source File

SOURCE=..\VBSRC\VBDATTIM.C
# End Source File
# Begin Source File

SOURCE=..\VBSRC\VBPRINT2.C
# End Source File
# Begin Source File

SOURCE=..\VBSRC\VBSPREA5.C
# End Source File
# Begin Source File

SOURCE=..\VBSRC\VBSSCNVT.CPP
# End Source File
# Begin Source File

SOURCE=..\web\src\web.cpp
# End Source File
# Begin Source File

SOURCE=..\SSSRC\WIN.C
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CLASSES\WINTOOLS.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\SSSRC\CALC\cal_cell.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\cal_col.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\cal_cust.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\calc\cal_err.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\calc\cal_expr.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_FUNC.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_MATR.H
# End Source File
# Begin Source File

SOURCE=..\sssrc\calc\cal_mem.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\cal_name.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_OPER.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\cal_row.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_TOK.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CALC\CAL_VECT.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\CALENDAR.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\classes\checkbox.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITDATE\EDITDATE.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\editfld.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITFLOA\EDITFLOA.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITGEN\EDITGEN.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITINT\EDITINT.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITPIC\editpic.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\EDIT\EDITTIME\EDITTIME.H
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CLASSES\SPINBTN.H
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\spread.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_alloc.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_bool.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_bord.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_calc.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_cell.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_col.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_curs.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_data.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_doc.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_DRAG.H
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_draw.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_EMODE.H
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_excel.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_font.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_formu.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_FUNC.H
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_html.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_main.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_multi.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_row.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_save.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_save4.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SCBAR.H
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_scrol.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_SPICT.H
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_type.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_user.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\spread\ss_virt.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\SPREAD\SS_WIN.H
# End Source File
# Begin Source File

SOURCE=.\Ssocx.h
# End Source File
# Begin Source File

SOURCE=.\Ssocxctl.h
# End Source File
# Begin Source File

SOURCE=.\ssocxdb.h
# End Source File
# Begin Source File

SOURCE=.\Ssocxppg.h
# End Source File
# Begin Source File

SOURCE=.\ssodl.h
# End Source File
# Begin Source File

SOURCE=.\Stdafx.h
# End Source File
# Begin Source File

SOURCE=..\SSSRC\CLASSES\superbtn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\FPLIBS\FPTOOLS\SRC\TEXTTIP.H
# End Source File
# Begin Source File

SOURCE=..\vbsrc\vbdattim.h
# End Source File
# Begin Source File

SOURCE=..\vbsrc\vbprint2.h
# End Source File
# Begin Source File

SOURCE=..\vbsrc\vbsprea5.h
# End Source File
# Begin Source File

SOURCE=..\vbsrc\vbsscnvt.h
# End Source File
# Begin Source File

SOURCE=.\Virtualp.h
# End Source File
# Begin Source File

SOURCE=..\sssrc\classes\wintools.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\Ssprvw\Pvctl.bmp
# End Source File
# Begin Source File

SOURCE=..\ssprvw\Pvctld.bmp
# End Source File
# Begin Source File

SOURCE=..\spread.ico
# End Source File
# Begin Source File

SOURCE=..\vbx\Spread.ico
# End Source File
# Begin Source File

SOURCE=.\Ssocx.dlg
# End Source File
# Begin Source File

SOURCE=.\Ssocx.ico
# End Source File
# Begin Source File

SOURCE=.\Ssocx.rc2
# End Source File
# Begin Source File

SOURCE=.\Ssocxctl.bmp
# End Source File
# Begin Source File

SOURCE=.\Ssocxd.bmp
# End Source File
# Begin Source File

SOURCE=..\ssprvw\zoomin.cur
# End Source File
# Begin Source File

SOURCE=..\Ssprvw\zoomout.cur
# End Source File
# End Group
# End Target
# End Project
