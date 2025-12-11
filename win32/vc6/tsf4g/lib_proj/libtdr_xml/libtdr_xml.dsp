# Microsoft Developer Studio Project File - Name="libtdr_xml" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libtdr_xml - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtdr_xml.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtdr_xml.mak" CFG="libtdr_xml - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libtdr_xml - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libtdr_xml - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libtdr_xml - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../../win32_lib/vc6/release"
# PROP Intermediate_Dir "..\..\..\..\..\..\tsf4g_output\libraries\tdr\libtdr_xml\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../../../deps/scew" /I "../../../../../deps/expat/lib" /I "../../../../../include" /I "../../../../../lib_src/" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "XML_STATIC" /D "TSF4G_STATIC" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"$(outdir)\libtdr_xml.lib"

!ELSEIF  "$(CFG)" == "libtdr_xml - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../win32_lib/vc6/debug"
# PROP Intermediate_Dir "..\..\..\..\..\..\tsf4g_output\libraries\tdr\libtdr_xml\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../../deps/scew" /I "../../../../../deps/expat/lib" /I "../../../../../include" /I "../../../../../lib_src/" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "TSF4G_STATIC" /D "XML_STATIC" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"$(outdir)\libtdr_xml_d.lib"

!ENDIF 

# Begin Target

# Name "libtdr_xml - Win32 Release"
# Name "libtdr_xml - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_auxtools.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_ctypes_info.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_data_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_data_sort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_error_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_iostream_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_md5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_meta_entries_index.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_metalib_init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_metalib_manage_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_net.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_operate_data.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_os.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_strptime.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_auxtools.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_ctypes_info_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_define_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_error_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_iostream_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_md5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_metalib_kernel_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_metalib_manage_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_net_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\tdr_os.h
# End Source File
# End Group
# Begin Group "xml_metalib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_data_inoutput.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_metaleb_param_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_metaleb_param_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_metalib_entry_manage_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_metalib_entry_manage_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_metalib_meta_manage_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_metalib_meta_manage_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_metalib_to_hpp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_scew_if.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_scew_if.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_XMLMetaLib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_XMLMetalib_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\lib_src\tdr\xml_metalib\tdr_XMLtags_i.h
# End Source File
# End Group
# Begin Group "External Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_ctypes_info.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_data_initialize.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_data_io.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_data_sort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_define.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_error.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_metalib_init.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_metalib_manage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_metalib_to_hpp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_net.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_sql.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_XMLMetaLib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\include\tdr\tdr_XMLtags.h
# End Source File
# End Group
# End Target
# End Project
