# Microsoft Developer Studio Project File - Name="CxImageCrtLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CxImageCrtLib - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CxImageCrtLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CxImageCrtLib.mak" CFG="CxImageCrtLib - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CxImageCrtLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CxImageCrtLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "CxImageCrtLib - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "CxImageCrtLib - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CxImageCrtLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../zlib" /I "../jasper/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "JAS_WIN_MSVC_BUILD" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CxImageCrtLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "../zlib" /I "../jasper/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "JAS_WIN_MSVC_BUILD" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CxImageCrtLib - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CxImageCrtLib___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "CxImageCrtLib___Win32_Unicode_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "UnicodeDebug"
# PROP Intermediate_Dir "UnicodeDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "../zlib" /I "../jasper/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "JAS_WIN_MSVC_BUILD" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "../zlib" /I "../jasper/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "JAS_WIN_MSVC_BUILD" /D "_UNICODE" /D "UNICODE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CxImageCrtLib - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CxImageCrtLib___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "CxImageCrtLib___Win32_Unicode_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "UnicodeRelease"
# PROP Intermediate_Dir "UnicodeRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../zlib" /I "../jasper/include" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "JAS_WIN_MSVC_BUILD" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "CxImageCrtLib - Win32 Release"
# Name "CxImageCrtLib - Win32 Debug"
# Name "CxImageCrtLib - Win32 Unicode Debug"
# Name "CxImageCrtLib - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\CxImage\FPIMAGES.CPP
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximabmp.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximadsp.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximaenc.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximaexif.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximage.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximagif.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximaico.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximaj2k.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximajas.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximajbg.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximajpg.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximalpha.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximalyr.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximamng.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximapal.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximapcx.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximapng.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximasel.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximatga.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximatif.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximatran.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximawbmp.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximawmf.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\ximawnd.cpp
# End Source File
# Begin Source File

SOURCE=..\CxImage\xmemfile.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\CxImage\CxImageDLL\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\cximage\xfile.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximabmp.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximadefs.h
# End Source File
# Begin Source File

SOURCE=..\cximage\xImage.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximagif.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximaico.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximaiter.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximaj2k.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximajas.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximajbg.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximajpg.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximamng.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximapcx.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximapng.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximatga.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximatif.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximawbmp.h
# End Source File
# Begin Source File

SOURCE=..\cximage\ximawmf.h
# End Source File
# Begin Source File

SOURCE=..\cximage\xiofile.h
# End Source File
# Begin Source File

SOURCE=..\cximage\xmemfile.h
# End Source File
# End Group
# End Target
# End Project
