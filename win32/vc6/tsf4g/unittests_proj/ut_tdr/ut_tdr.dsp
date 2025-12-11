# Microsoft Developer Studio Project File - Name="ut_tdr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ut_tdr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ut_tdr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ut_tdr.mak" CFG="ut_tdr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ut_tdr - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ut_tdr - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ut_tdr - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\src\tdr\unittest"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\src\tdr\unittest\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../../include" /I "../../../..../../../../include" /I "../../../../include/depend" /I "../../../../src/tdr" /I "../../../../src/tdr/xml_metalib" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "TDR_WITH_DBMS" /D "TDR_WITH_XML" /D "XML_STATIC" /YX /FD /Zm800 /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"../../../../lib" /libpath:"../../../../deps/mysql/lib/debug"

!ELSEIF  "$(CFG)" == "ut_tdr - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\src\tdr\unittest"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\src\tdr\unittest\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../include" /I "../../../../src/tdr" /I "../../../../src/tdr/xml_metalib" /I "../../../../deps/scew" /I "../../../../deps/expat/lib" /I "../../../../deps/CUnit/CUnit/Headers" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "TDR_WITH_DBMS" /D "TDR_WITH_XML" /D "XML_STATIC" /YX /FD /GZ /Zm800 /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../../lib" /libpath:"../../../../deps/mysql/lib/debug"

!ENDIF 

# Begin Target

# Name "ut_tdr - Win32 Release"
# Name "ut_tdr - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\CUnitTest.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ov_cs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\tgetopt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_meta_index.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_metalib_to_cfile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_ormapping.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_tdr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_creat_metalib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_ctype_info.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_data_initialize.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_data_sort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_date_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_defaultvalue.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_md5hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_metalib_manage.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_metalib_net.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_metalib_to_hpp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_save_metalib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_tdr_db.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_tdr_iostream.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_tdrerror.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ov_cs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\simpleMetalibV0.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\simpleMetalibV0_spec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\simpleMetalibV1.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\simpleMetalibV1_spec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\simpleMetalibV1_spec1.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\simpleMetalibV2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\simpleMetalibV3.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\test_tdr_sort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\tgetopt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_tdr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_tdr_db.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_tdr_sort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\unittests\tdr\ut_test_defaultvalue.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
