# Microsoft Developer Studio Project File - Name="FPTOOL32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=FPTOOL32 - Win32 ANSI Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Fptool32_src.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fptool32_src.mak" CFG="FPTOOL32 - Win32 ANSI Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FPTOOL32 - Win32 ANSI Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "FPTOOL32 - Win32 ANSI Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FPTOOL32 - Win32 ANSI Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\objd32"
# PROP BASE Intermediate_Dir ".\objd32"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\objd32"
# PROP Intermediate_Dir ".\objd32"
# ADD BASE CPP /nologo /Zp1 /MT /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MEM_DEBUG" /D "FP_OCX" /D "NO_VBX_SUPPORT" /D "_MBCS" /FR /YX"fptools.h" /c
# ADD CPP /nologo /MD /W3 /GR /GX /ZI /Od /I ".\" /I "..\src" /D "_DEBUG" /D "FP_APARTMENT" /D "WIN32" /D "_WINDOWS" /D "FP_OCX" /D "NO_VBX_SUPPORT" /D "_MBCS" /D "VC50" /D "VC40" /D "NO_MFC" /D "CS_OFF" /D "NO_SMARTHEAP" /YX"fptools.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\objd32\fptools.lib"

!ELSEIF  "$(CFG)" == "FPTOOL32 - Win32 ANSI Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\obj32"
# PROP BASE Intermediate_Dir ".\obj32"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\obj32"
# PROP Intermediate_Dir ".\obj32"
# ADD BASE CPP /nologo /Zp1 /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "FP_OCX" /D "NO_VBX_SUPPORT" /D "_MBCS" /FR /YX"fptools.h" /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I ".\" /I "..\src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "FP_OCX" /D "NO_VBX_SUPPORT" /D "_MBCS" /D "VC50" /D "VC40" /D "NO_MFC" /D "FP_APARTMENT" /D "CS_OFF" /D "NO_SMARTHEAP" /YX"fptools.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\obj32\fptools.lib"

!ENDIF 

# Begin Target

# Name "FPTOOL32 - Win32 ANSI Debug"
# Name "FPTOOL32 - Win32 ANSI Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\SRC\FPABOUT.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPASSERT.C
# End Source File
# Begin Source File

SOURCE=.\Fpatl.cpp
# End Source File
# Begin Source File

SOURCE=.\Fpatlcor.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\FPCONV1.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPCONV2.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPCONV3.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPCONV4.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPCONV5.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPCONV6.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPCONV7.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\fpconv8.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\FPDATE.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPDIB.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPDT.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPFILE.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPGRAPHC.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPHELP.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPHUGE.C
# End Source File
# Begin Source File

SOURCE=..\Src\fpload.c
# End Source File
# Begin Source File

SOURCE=..\SRC\FPMEMMGR.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPNCPNT.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPNCPROP.C
# End Source File
# Begin Source File

SOURCE=..\SRC\Fpocxpp.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Fppict.c
# End Source File
# Begin Source File

SOURCE=..\Src\fpsave.c
# End Source File
# Begin Source File

SOURCE=..\SRC\FPSTDPRP.C
# End Source File
# Begin Source File

SOURCE=..\SRC\fpstrtod.c
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTIME.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL01.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL02.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL03.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL04.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL05.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL06.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL07.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL08.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL09.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL10.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL11.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL12.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL14.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL15.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL16.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL17.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL19.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL20.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL21.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL22.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL23.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL24.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL25.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL26.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL27.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL29.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL30.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL31.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL32.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL40.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL41.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL42.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL43.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL44.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL45.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL46.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL47.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL50.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL51.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL52.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL53.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL54.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL55.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL56.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL57.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL58.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL59.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL60.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL61.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTOOL62.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPTSTR.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPVBX2.C
# End Source File
# Begin Source File

SOURCE=..\SRC\FPWAPI1.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPWAPI2.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPWAPI3.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPWAPI4.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\FPWAPI5.CPP
# End Source File
# Begin Source File

SOURCE=..\SRC\fpwapi6.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Fpxprop.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\GLOBALS.CPP
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\Util.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\VBNCPNT.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\SRC\fphuge.h
# End Source File
# Begin Source File

SOURCE=..\SRC\fpidptbl.h
# End Source File
# Begin Source File

SOURCE=..\src\fpmemmgr.h
# End Source File
# Begin Source File

SOURCE=..\SRC\FPNCPNT.H
# End Source File
# Begin Source File

SOURCE=..\SRC\FPNCPROP.H
# End Source File
# Begin Source File

SOURCE=..\SRC\fpocxpp.h
# End Source File
# Begin Source File

SOURCE=..\Src\fpsave.h
# End Source File
# Begin Source File

SOURCE=..\SRC\FPSTDPRP.H
# End Source File
# Begin Source File

SOURCE=..\SRC\fptstr.h
# End Source File
# Begin Source File

SOURCE=..\SRC\fpxprop.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
