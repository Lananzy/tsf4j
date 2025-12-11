# Microsoft Developer Studio Project File - Name="cus_msg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=cus_msg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cus_msg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cus_msg.mak" CFG="cus_msg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cus_msg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cus_msg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cus_msg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\apps\clientupdatesystem\lib\vc6\release"
# PROP Intermediate_Dir "..\..\..\..\..\..\tsf4g_out\clientupdatesystem\msg\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\..\..\apps\clientupdatesystem\include" /I "..\..\..\..\..\include" /I "..\..\..\..\..\lib_src" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "CUS_STATIC" /D "TDR_WITH_XML" /D "TSF4G_STATIC" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\apps\clientupdatesystem\lib\vc6\release\cus_protocol.lib"

!ELSEIF  "$(CFG)" == "cus_msg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\apps\clientupdatesystem\lib\vc6\debug"
# PROP Intermediate_Dir "..\..\..\..\..\..\tsf4g_out\clientupdatesystem\msg\debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\..\..\apps\clientupdatesystem\include" /I "..\..\..\..\..\include" /I "..\..\..\..\..\lib_src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "CUS_STATIC" /D "TDR_WITH_XML" /D "TSF4G_STATIC" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\apps\clientupdatesystem\lib\vc6\debug\cus_protocol_d.lib"

!ENDIF 

# Begin Target

# Name "cus_msg - Win32 Release"
# Name "cus_msg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\msg\cus_business_data.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\msg\cus_net_msg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\msg\cus_protocol_lib.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\msg\cus_business_data.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\include\cus_net_msg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\include\cus_platform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\include\cus_protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\apps\clientupdatesystem\include\cus_public.h
# End Source File
# End Group
# End Target
# End Project
