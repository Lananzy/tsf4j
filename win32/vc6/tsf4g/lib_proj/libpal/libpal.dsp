# Microsoft Developer Studio Project File - Name="libpal" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libpal - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libpal.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libpal.mak" CFG="libpal - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libpal - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libpal - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libpal - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../../win32_lib/vc6/release"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\libraries\pal\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../../../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libpal - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../win32_lib/vc6/debug"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\libraries\pal\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../../../win32_lib/vc6/debug\libpal_d.lib"

!ENDIF 

# Begin Target

# Name "libpal - Win32 Release"
# Name "libpal - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\dirent.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\fcntl.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\getopt.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_attr.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_barrier.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_barrierattr.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_cond.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_condattr.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_mutexattr.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_rwlock.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_rwlockattr.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\pthread_spin.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\semaphore.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\shortcut.cpp
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\strptime.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\strtoll.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\strtoull.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tdir.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\terrno.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\texec.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tfile.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tfork.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tipc.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tlock.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tmmap.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tmsgque.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tnet.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tos.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tpoll.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tresource.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tsched.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tsem.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tshm.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tsignal.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tsocket.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tstdio.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tstdlib.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tthread.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\ttime.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\tuio.c
# End Source File
# Begin Source File

SOURCE=\tsf4g\lib_src\pal\twait.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=\tsf4g\include\pal\err.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\getopt.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\pal.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\pcreposix.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\queue.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\siginfo.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tapi_external.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tdir.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tdirent.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tdlfcn.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\terrno.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\texec.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tfcntl.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tfile.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tfork.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tinet.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tipc.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tlock.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tmmap.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tmsgque.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tmutex.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tnet.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tos.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tpoll.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tresource.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\trules.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tsafecode.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tsched.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tsem.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tsemaphore.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tshm.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tsignal.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tsocket.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tstdio.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tstdlib.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tstring.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tthread.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\ttime.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\ttypes.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\tuio.h
# End Source File
# Begin Source File

SOURCE=\tsf4g\include\pal\twait.h
# End Source File
# End Group
# End Target
# End Project
