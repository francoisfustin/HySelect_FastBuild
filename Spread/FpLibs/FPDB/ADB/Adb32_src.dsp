# Microsoft Developer Studio Project File - Name="Adb32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Adb32 - Win32 ANSI Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Adb32_src.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Adb32_src.mak" CFG="Adb32 - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Adb32 - Win32 ANSI Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Adb32 - Win32 ANSI Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Adb32 - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Adb32 - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Adb32 - Win32 ANSI Debug"

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
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\" /I "..\src" /I "..\..\fptools\src" /I "..\..\fptools\atl" /I "..\..\msdasdk\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NO_MFC" /D "VC50" /D "FP_ADB" /D "FP_OCX" /D "__OCDB_H_" /D "DEBUG" /D "NO_SH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\objd32\Adb32.lib"

!ELSEIF  "$(CFG)" == "Adb32 - Win32 ANSI Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\" /I /I "..\src" "..\..\fptools\src" /I "..\..\fptools\atl" /I "..\..\msdasdk\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NO_MFC" /D "VC50" /D "FP_ADB" /D "FP_OCX" /D "__OCDB_H_" /D "NO_SH" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\obj32\Adb32.lib"

!ELSEIF  "$(CFG)" == "Adb32 - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\objud32"
# PROP BASE Intermediate_Dir ".\objud32"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\objud32"
# PROP Intermediate_Dir ".\objud32"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\" /I "..\src" /I "..\..\fpdb\adb" /I "..\..\fptools\src" /I "..\..\fptools\atl" /I "..\..\msdasdk\include" /D "_DEBUG" /D "_UNICODE" /D "UNICODE" /D "WIN32" /D "_WINDOWS" /D "NO_MFC" /D "VC50" /D "FP_ADB" /D "FP_OCX" /D "__OCDB_H_" /D "DEBUG" /D "NO_SH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\objud32\Adb32.lib"

!ELSEIF  "$(CFG)" == "Adb32 - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\obju32"
# PROP BASE Intermediate_Dir ".\obju32"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\obju32"
# PROP Intermediate_Dir ".\obju32"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\" /I "..\src" /I "..\..\fptools\src" /I "..\..\fptools\atl" /I "..\..\msdasdk\include" /D "NDEBUG" /D "_UNICODE" /D "UNICODE" /D "WIN32" /D "_WINDOWS" /D "NO_MFC" /D "VC50" /D "FP_ADB" /D "FP_OCX" /D "__OCDB_H_" /D "NO_SH" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\obju32\Adb32.lib"

!ENDIF 

# Begin Target

# Name "Adb32 - Win32 ANSI Debug"
# Name "Adb32 - Win32 ANSI Release"
# Name "Adb32 - Win32 Unicode Debug"
# Name "Adb32 - Win32 Unicode Release"
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\Fpadb.cpp
# End Source File
# Begin Source File

SOURCE=.\fpdbacal.cpp
# End Source File
# Begin Source File

SOURCE=.\FPDBCOLA.CPP
# End Source File
# Begin Source File

SOURCE=.\fpdbevta.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\fpdblist.cpp
# End Source File
# Begin Source File

SOURCE=.\fpdbsupp.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\FPDBUTIL.CPP
# End Source File
# Begin Source File

SOURCE=.\Fpoledba.cpp
# End Source File
# Begin Source File

SOURCE=.\unknown.cpp
# End Source File
# End Target
# End Project
