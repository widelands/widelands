# Microsoft Developer Studio Project File - Name="wwfcreate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=wwfcreate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wwfcreate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wwfcreate.mak" CFG="wwfcreate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wwfcreate - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "wwfcreate - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wwfcreate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wwfcreate___Win32_Release"
# PROP BASE Intermediate_Dir "wwfcreate___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".."
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /TP /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib sdl.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "wwfcreate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wwfcreate___Win32_Debug"
# PROP BASE Intermediate_Dir "wwfcreate___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".."
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /TP /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib sdl.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "wwfcreate - Win32 Release"
# Name "wwfcreate - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;cc"
# Begin Source File

SOURCE=..\wtfcreate\bob_descr.cc
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\file_descr.cc
# End Source File
# Begin Source File

SOURCE=..\src\growablearray.cc
# End Source File
# Begin Source File

SOURCE=..\src\helper.cc
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\logic_bob_descr.cc
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\main.cc
# End Source File
# Begin Source File

SOURCE=..\src\md5.cc
# End Source File
# Begin Source File

SOURCE=..\src\md5file.cc
# End Source File
# Begin Source File

SOURCE=..\src\mydirent.cc
# End Source File
# Begin Source File

SOURCE=..\src\myfile.cc
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\need_list.cc
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\parse.cc
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\parse_bobs.cc
# End Source File
# Begin Source File

SOURCE=..\src\pic.cc
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\pic_descr.cc
# End Source File
# Begin Source File

SOURCE=..\src\profile.cc
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\resource_descr.cc
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\terrain_descr.cc
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\write_worldfile.cc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\wtfcreate\bob_descr.h
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\file_descr.h
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\logic_bob_descr.h
# End Source File
# Begin Source File

SOURCE=..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\src\md5file.h
# End Source File
# Begin Source File

SOURCE=..\src\myfile.h
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\need_list.h
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\parse.h
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\parse_bobs.h
# End Source File
# Begin Source File

SOURCE=..\wtfcreate\pic_descr.h
# End Source File
# Begin Source File

SOURCE=..\src\profile.h
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\resource_descr.h
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\terrain_descr.h
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\write_worldfile.h
# End Source File
# Begin Source File

SOURCE=..\wwfcreate\wwfcreate.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
