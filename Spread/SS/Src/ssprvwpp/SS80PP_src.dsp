# Microsoft Developer Studio Project File - Name="SS80PP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SS80PP - Win32 ANSI Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SS80PP_src.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SS80PP_src.mak" CFG="SS80PP - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SS80PP - Win32 ANSI Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SS80PP - Win32 ANSI Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SS80PP - Win32 ANSI Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\objd32"
# PROP BASE Intermediate_Dir ".\objd32"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\objd32"
# PROP Intermediate_Dir ".\objd32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "..\..\..\fplibs\htmlhelp" /I "..\..\..\fplibs\fptools\atl" /I "..\..\..\fplibs\fptools\src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "VC50" /D "NO_MFC" /D "FP_OCX" /D "HTMLHELP" /D "SS_V40" /D "SS_V70" /D "SS_V80" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /D "SS_V35" /D "SS_V40" /D "SS_V70" /D "SS_V80" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "SS_V35" /d "SS_V40" /d "SS_V70" /d "SS_V80"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fptools.lib atl.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib htmlhelp.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBCD" /nodefaultlib:"MSVCRT" /nodefaultlib:"MSVCRTD" /out:".\objd32\SS80PP.dll" /pdbtype:sept /libpath:"..\..\..\fplibs\fptools\atl\objd32\" /libpath:"..\..\..\fplibs\htmlhelp"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\objd32
TargetPath=.\objd32\SS80PP.dll
InputPath=.\objd32\SS80PP.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "SS80PP - Win32 ANSI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\obj32"
# PROP BASE Intermediate_Dir ".\obj32"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\obj32"
# PROP Intermediate_Dir ".\obj32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /O1 /I "..\..\..\fplibs\htmlhelp" /I "..\..\..\fplibs\fptools\atl" /I "..\..\..\fplibs\fptools\src" /D "NDEBUG" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "VC50" /D "NO_MFC" /D "FP_OCX" /D "HTMLHELP" /D "SS_V40" /D "SS_V70" /D "SS_V80" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /D "SS_V35" /D "SS_V40" /D "SS_V70" /D "SS_V80" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"Release80/SS80PP.res" /d "NDEBUG" /d "SS_V35" /d "SS_V40" /d "SS_V70" /d "SS_V80"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 fptools.lib atl.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib htmlhelp.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\obj32\SS80PP.dll" /libpath:"..\..\..\fplibs\fptools\atl\obj32\"  /libpath:"..\..\..\fplibs\htmlhelp"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\obj32
TargetPath=.\obj32\SS80PP.dll
InputPath=.\obj32\SS80PP.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "SS80PP - Win32 ANSI Debug"
# Name "SS80PP - Win32 ANSI Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\FPLIBS\FPTOOLS\ATL\Atlppbas.cpp
# End Source File
# Begin Source File

SOURCE=.\PPGeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\PPPage.cpp
# End Source File
# Begin Source File

SOURCE=.\SS30PP.cpp
# End Source File
# Begin Source File

SOURCE=.\SS30PP.idl

!IF  "$(CFG)" == "SS80PP - Win32 ANSI Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing MIDL step
InputPath=.\SS30PP.idl

BuildCmds= \
	midl /Oicf /D "SS_V40" /D "SS_V70" /D "SS_V80" /h "SS30PP.h" /iid "SS30PP_i.c" "SS30PP.idl"

".\SS30PP.tlb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\SS30PP.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\SS30PP_i.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "SS80PP - Win32 ANSI Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing MIDL step
InputPath=.\SS30PP.idl

BuildCmds= \
	midl /Oicf /D "SS_V40" /D "SS_V70" /D "SS_V80" /h "SS30PP.h" /iid "SS30PP_i.c" "SS30PP.idl"

".\SS30PP.tlb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\SS30PP.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\SS30PP_i.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SS30PP.rc
# End Source File
# Begin Source File

SOURCE=.\SS80PP.def
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\PPGeneral.h
# End Source File
# Begin Source File

SOURCE=.\PPPage.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\PPGeneral80.rgs
# End Source File
# Begin Source File

SOURCE=.\PPPage80.rgs
# End Source File
# End Target
# End Project
