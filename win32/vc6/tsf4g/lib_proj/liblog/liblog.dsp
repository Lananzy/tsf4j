# Microsoft Developer Studio Project File - Name="liblog" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=liblog - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "liblog.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "liblog.mak" CFG="liblog - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "liblog - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "liblog - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "liblog - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../lib"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\libraries\log\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBLOG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../../../include" /I "../../../../include/log" /I "../../../../include/log/sd" /I "../../../../deps/expat/lib" /I "../../../../include/pal" /I "GNORE:4089" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBLOG_EXPORTS" /D "HAVE_CONFIG_H" /D "LOG4C_EXPORTS" /D "_WIN32" /D "XML_STATIC" /YX /FD /D /NODEFAULTLIB:libcmt.lib" " /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT_d.lib libpal.lib Ws2_32.lib Winmm.lib /nologo /dll /incremental:yes /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /nodefaultlib:"msvcrtd.lib" /libpath:"../../../../lib"

!ELSEIF  "$(CFG)" == "liblog - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../lib"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\libraries\log\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBLOG_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../include" /I "../../../../include/log" /I "../../../../include/log/sd" /I "../../../../deps/expat/lib" /I "../../../../include/pal" /D "_DEBUG" /D "HAVE_CONFIG_H" /D "LOG4C_EXPORTS" /D "_WIN32" /D "XML_STATIC" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBLOG_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libexpatMT_d.lib libpal.lib Ws2_32.lib Winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"../../../../lib"

!ENDIF 

# Begin Target

# Name "liblog - Win32 Release"
# Name "liblog - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\libraries\log\appender.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\appender_type_mmap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\appender_type_rollingfile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\appender_type_stream.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\appender_type_stream2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\appender_type_syslog.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\category.c
# End Source File
# Begin Source File

SOURCE="..\..\..\..\libraries\log\sd\domnode-expat.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\libraries\log\sd\domnode-xml-parser.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\libraries\log\sd\domnode-xml-scanner.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\libraries\log\sd\domnode-xml.c"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\factory.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\layout.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\layout_type_basic.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\layout_type_basic_r.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\layout_type_dated.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\layout_type_dated_r.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\list.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\logging_event.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\malloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\priority.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\rc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\rollingpolicy.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\rollingpolicy_type_sizewin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\sd_xplatform.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\sprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\stack.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\sd\test.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\log\version.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\include\log\appender.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\appender_type_mmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\appender_type_rollingfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\appender_type_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\appender_type_stream2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\appender_type_syslog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\category.h
# End Source File
# Begin Source File

SOURCE="..\..\..\..\include\log\config-win32.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\defs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\defs.h
# End Source File
# Begin Source File

SOURCE="..\..\..\..\include\log\sd\domnode-xml-parser.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\include\log\sd\domnode-xml-scanner.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\include\log\sd\domnode-xml.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\domnode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\error.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\factory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\init.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\layout.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\layout_type_basic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\layout_type_basic_r.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\layout_type_dated.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\layout_type_dated_r.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\list.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\location_info.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\log4c.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\logging_event.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\malloc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\priority.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\rc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\rollingpolicy.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\rollingpolicy_type_sizewin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\sd_xplatform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\sprintf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\stack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\sd\test.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\log\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
