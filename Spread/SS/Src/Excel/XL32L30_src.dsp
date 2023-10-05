# Microsoft Developer Studio Project File - Name="XL32L30" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=XL32L30 - Win32 ANSI Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "XL32L30_src.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "XL32L30_src.mak" CFG="XL32L30 - Win32 ANSI Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "XL32L30 - Win32 ANSI Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "XL32L30 - Win32 ANSI Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "XL32L30 - Win32 ANSI Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\objd32"
# PROP BASE Intermediate_Dir ".\objd32"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\objd32"
# PROP Intermediate_Dir ".\objd32"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /GR /GX /Zi /Od /I "..\sssrc\spread" /I "..\..\include" /I "..\..\..\fplibs\fptools\src" /D "SS_DLL" /D "VC60" /D "SS_V30" /D "_DEBUG" /D "FP_DLL" /D "WIN32" /D "_WINDOWS" /D "VC50" /D "SS_NO_USE_SH" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\objd32\XL32L30.lib"

!ELSEIF  "$(CFG)" == "XL32L30 - Win32 ANSI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\obj32"
# PROP BASE Intermediate_Dir ".\obj32"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\obj32"
# PROP Intermediate_Dir ".\obj32"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /I "..\sssrc\spread" /I "..\..\include" /I "..\..\..\fplibs\fptools\src" /D "NDEBUG" /D "SS_DLL" /D "SS_BOUNDCONTROL" /D "FP_DLL" /D "WIN32" /D "_WINDOWS" /D "VC50" /D "VC60" /D "SS_V30" /D "SS_NO_USE_SH" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\obj32\XL32L30.lib"

!ENDIF 

# Begin Target

# Name "XL32L30 - Win32 ANSI Debug"
# Name "XL32L30 - Win32 ANSI Release"
# Begin Source File

SOURCE=src\biff.h
# End Source File
# Begin Source File

SOURCE=src\xl.cpp
# End Source File
# Begin Source File

SOURCE=src\xl.h
# End Source File
# Begin Source File

SOURCE=src\xlbiff.cpp
# End Source File
# Begin Source File

SOURCE=src\xlbiff.h
# End Source File
# Begin Source File

SOURCE=Src\xlloadl.cpp
# End Source File
# Begin Source File

SOURCE=src\xlssm.cpp
# End Source File
# Begin Source File

SOURCE=src\xlutl.cpp
# End Source File
# Begin Source File

SOURCE=Src\xlutl.h
# End Source File
# End Target
# End Project
