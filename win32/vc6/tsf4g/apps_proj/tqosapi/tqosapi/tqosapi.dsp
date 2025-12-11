# Microsoft Developer Studio Project File - Name="tqosapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=tqosapi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tqosapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tqosapi.mak" CFG="tqosapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tqosapi - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "tqosapi - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tqosapi - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\..\..\..\include" /I "..\..\..\..\..\..\apps\tqos\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "TDR_WITH_XML" /D "TSF4G_STATIC" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\..\win32_lib\vc6\release\tqosapi.lib"

!ELSEIF  "$(CFG)" == "tqosapi - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\..\..\include" /I "..\..\..\..\..\..\apps\tqos\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "TDR_WITH_XML" /D "TSF4G_STATIC" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\..\win32_lib\vc6\debug\tqosapi_d.lib"

!ENDIF 

# Begin Target

# Name "tqosapi - Win32 Release"
# Name "tqosapi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=\tsf4g\apps\tqos\tqosapi\tqos.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\apps\tqos\tqosapi\tqos_cs.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\apps\tqos\tqosapi\tqos_ss.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=\tsf4g\apps\tqos\include\tqos.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\apps\tqos\include\tqos_cs.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\apps\tqos\include\tqos_ss.h
# End Source File
# End Group
# End Target
# End Project
