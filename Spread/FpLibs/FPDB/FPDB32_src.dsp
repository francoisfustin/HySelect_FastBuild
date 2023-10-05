# Microsoft Developer Studio Project File - Name="FPDB32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=FPDB32 - Win32 ANSI Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FPDB32_src.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FPDB32_src.mak" CFG="FPDB32 - Win32 ANSI Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FPDB32 - Win32 ANSI Release" (based on "Win32 (x86) Static Library")
!MESSAGE "FPDB32 - Win32 ANSI Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "FPDB32 - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE "FPDB32 - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FPDB32 - Win32 ANSI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\obj32"
# PROP BASE Intermediate_Dir ".\obj32"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\obj32"
# PROP Intermediate_Dir ".\obj32"
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\fptools\src" /I ".\src" /D "NDEBUG" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "FP_OCX" /D "_MBCS" /D "VC50" /D "VC40" /D "NO_SH" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\obj32\FPDB32.lib"

!ELSEIF  "$(CFG)" == "FPDB32 - Win32 ANSI Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\objd32"
# PROP BASE Intermediate_Dir ".\objd32"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\objd32"
# PROP Intermediate_Dir ".\objd32"
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\fptools\src" /I ".\src" /D "_DEBUG" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "FP_OCX" /D "_MBCS" /D "VC50" /D "VC40" /D "NO_SH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\objd32\FPDB32.lib"

!ELSEIF  "$(CFG)" == "FPDB32 - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\obju32"
# PROP BASE Intermediate_Dir ".\obju32"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\obju32"
# PROP Intermediate_Dir ".\obju32"
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\fptools\src" /I ".\src" /D "NDEBUG" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "FP_OCX" /D "VC50" /D "VC40" /D "NO_SH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\obju32\FPDB32.lib"

!ELSEIF  "$(CFG)" == "FPDB32 - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\objud32"
# PROP BASE Intermediate_Dir ".\objud32"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\objud32"
# PROP Intermediate_Dir ".\objud32"
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\fptools\src" /I ".\src" /D "_DEBUG" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "FP_OCX" /D "VC50" /D "VC40" /D "NO_SH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\objud32\FPDB32.lib"

!ENDIF 

# Begin Target

# Name "FPDB32 - Win32 ANSI Release"
# Name "FPDB32 - Win32 ANSI Debug"
# Name "FPDB32 - Win32 Unicode Release"
# Name "FPDB32 - Win32 Unicode Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\SRC\FPDBCALL.CPP
# End Source File
# Begin Source File

SOURCE=.\SRC\FPDBCOL.CPP
# End Source File
# Begin Source File

SOURCE=.\SRC\FPDBEVNT.CPP
# End Source File
# Begin Source File

SOURCE=.\SRC\FPDBNOTE.CPP
# End Source File
# Begin Source File

SOURCE=.\SRC\FPDBUTIL.CPP
# End Source File
# Begin Source File

SOURCE=.\SRC\FPOLEDB.CPP
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\SRC\FPDBCOL.H
# End Source File
# Begin Source File

SOURCE=.\Src\Fpdbnote.h
# End Source File
# Begin Source File

SOURCE=.\SRC\FPDBSIMP.H
# End Source File
# Begin Source File

SOURCE=.\Src\Fpdbutil.h
# End Source File
# Begin Source File

SOURCE=.\SRC\FPOLEDB.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
