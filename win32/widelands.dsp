# Microsoft Developer Studio Project File - Name="widelands" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=widelands - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "widelands.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "widelands.mak" CFG="widelands - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "widelands - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "widelands - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "widelands - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".."
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /TP /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib sdl.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "widelands - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".."
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /TP /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib sdl.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "widelands - Win32 Release"
# Name "widelands - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;cc"
# Begin Source File

SOURCE=..\src\auto_pic.cc
# End Source File
# Begin Source File

SOURCE=..\src\bob.cc
# End Source File
# Begin Source File

SOURCE=..\src\building.cc
# End Source File
# Begin Source File

SOURCE=..\src\button.cc
# End Source File
# Begin Source File

SOURCE=..\src\checkbox.cc
# End Source File
# Begin Source File

SOURCE=..\src\cmd_queue.cc
# End Source File
# Begin Source File

SOURCE=..\src\counter.cc
# End Source File
# Begin Source File

SOURCE=..\src\creature.cc
# End Source File
# Begin Source File

SOURCE=..\src\criterr.cc
# End Source File
# Begin Source File

SOURCE=..\src\cursor.cc
# End Source File
# Begin Source File

SOURCE=..\src\field.cc
# End Source File
# Begin Source File

SOURCE=..\src\fieldaction.cc
# End Source File
# Begin Source File

SOURCE=..\src\fileloc.cc
# End Source File
# Begin Source File

SOURCE=..\src\font.cc
# End Source File
# Begin Source File

SOURCE=..\src\game.cc
# End Source File
# Begin Source File

SOURCE=..\src\graphic.cc
# End Source File
# Begin Source File

SOURCE=..\src\growablearray.cc
# End Source File
# Begin Source File

SOURCE=..\src\helper.cc
# End Source File
# Begin Source File

SOURCE=..\src\input.cc
# End Source File
# Begin Source File

SOURCE=..\src\instances.cc
# End Source File
# Begin Source File

SOURCE=..\src\IntPlayer.cc
# End Source File
# Begin Source File

SOURCE=..\src\intro.cc
# End Source File
# Begin Source File

SOURCE=..\src\listselect.cc
# End Source File
# Begin Source File

SOURCE=..\src\main.cc
# End Source File
# Begin Source File

SOURCE=..\src\mainmenue.cc
# End Source File
# Begin Source File

SOURCE=..\src\map.cc
# End Source File
# Begin Source File

SOURCE=..\src\mapselectmenue.cc
# End Source File
# Begin Source File

SOURCE=..\src\mapview.cc
# End Source File
# Begin Source File

SOURCE=..\src\md5.cc
# End Source File
# Begin Source File

SOURCE=..\src\md5file.cc
# End Source File
# Begin Source File

SOURCE=..\src\menuecommon.cc
# End Source File
# Begin Source File

SOURCE=..\src\minimap.cc
# End Source File
# Begin Source File

SOURCE=..\src\multiline_textarea.cc
# End Source File
# Begin Source File

SOURCE=..\src\mydirent.cc
# End Source File
# Begin Source File

SOURCE=..\src\myfile.cc
# End Source File
# Begin Source File

SOURCE=..\src\options.cc
# End Source File
# Begin Source File

SOURCE=..\src\optionsmenu.cc
# End Source File
# Begin Source File

SOURCE=..\src\parser.cc
# End Source File
# Begin Source File

SOURCE=..\src\pic.cc
# End Source File
# Begin Source File

SOURCE=..\src\player.cc
# End Source File
# Begin Source File

SOURCE=..\src\profile.cc
# End Source File
# Begin Source File

SOURCE=..\src\radiobutton.cc
# End Source File
# Begin Source File

SOURCE=..\src\s2map.cc
# End Source File
# Begin Source File

SOURCE=..\src\scrollbar.cc
# End Source File
# Begin Source File

SOURCE=..\src\setup.cc
# End Source File
# Begin Source File

SOURCE=..\src\singlepmenue.cc
# End Source File
# Begin Source File

SOURCE=..\src\textarea.cc
# End Source File
# Begin Source File

SOURCE=..\src\tribe.cc
# End Source File
# Begin Source File

SOURCE=..\src\ui.cc
# End Source File
# Begin Source File

SOURCE=..\src\ware.cc
# End Source File
# Begin Source File

SOURCE=..\src\watchwindow.cc
# End Source File
# Begin Source File

SOURCE=..\src\window.cc
# End Source File
# Begin Source File

SOURCE=..\src\worker.cc
# End Source File
# Begin Source File

SOURCE=..\src\world.cc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\auto_pic.h
# End Source File
# Begin Source File

SOURCE=..\src\bob.h
# End Source File
# Begin Source File

SOURCE=..\src\building.h
# End Source File
# Begin Source File

SOURCE=..\src\cmd_queue.h
# End Source File
# Begin Source File

SOURCE=..\src\config.h
# End Source File
# Begin Source File

SOURCE=..\src\counter.h
# End Source File
# Begin Source File

SOURCE=..\src\creature.h
# End Source File
# Begin Source File

SOURCE=..\src\cursor.h
# End Source File
# Begin Source File

SOURCE=..\src\descr_maintainer.h
# End Source File
# Begin Source File

SOURCE=..\src\field.h
# End Source File
# Begin Source File

SOURCE=..\src\fieldaction.h
# End Source File
# Begin Source File

SOURCE=..\src\fileloc.h
# End Source File
# Begin Source File

SOURCE=..\src\font.h
# End Source File
# Begin Source File

SOURCE=..\src\game.h
# End Source File
# Begin Source File

SOURCE=..\src\graphic.h
# End Source File
# Begin Source File

SOURCE=..\src\growablearray.h
# End Source File
# Begin Source File

SOURCE=..\src\helper.h
# End Source File
# Begin Source File

SOURCE=..\src\input.h
# End Source File
# Begin Source File

SOURCE=..\src\instances.h
# End Source File
# Begin Source File

SOURCE=..\src\IntPlayer.h
# End Source File
# Begin Source File

SOURCE=..\src\intro.h
# End Source File
# Begin Source File

SOURCE=..\src\mainmenue.h
# End Source File
# Begin Source File

SOURCE=..\src\map.h
# End Source File
# Begin Source File

SOURCE=..\src\mapselectmenue.h
# End Source File
# Begin Source File

SOURCE=..\src\mapview.h
# End Source File
# Begin Source File

SOURCE=..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\src\md5file.h
# End Source File
# Begin Source File

SOURCE=..\src\menuecommon.h
# End Source File
# Begin Source File

SOURCE=..\src\minimap.h
# End Source File
# Begin Source File

SOURCE=..\src\mydirent.h
# End Source File
# Begin Source File

SOURCE=..\src\myfile.h
# End Source File
# Begin Source File

SOURCE=..\src\options.h
# End Source File
# Begin Source File

SOURCE=..\src\optionsmenu.h
# End Source File
# Begin Source File

SOURCE=..\src\parser.h
# End Source File
# Begin Source File

SOURCE=..\src\pic.h
# End Source File
# Begin Source File

SOURCE=..\src\player.h
# End Source File
# Begin Source File

SOURCE=..\src\profile.h
# End Source File
# Begin Source File

SOURCE=..\src\s2map.h
# End Source File
# Begin Source File

SOURCE=..\src\setup.h
# End Source File
# Begin Source File

SOURCE=..\src\singlepmenue.h
# End Source File
# Begin Source File

SOURCE=..\src\singleton.h
# End Source File
# Begin Source File

SOURCE=..\src\tribe.h
# End Source File
# Begin Source File

SOURCE=..\src\tribedata.h
# End Source File
# Begin Source File

SOURCE=..\src\ui.h
# End Source File
# Begin Source File

SOURCE=..\src\ware.h
# End Source File
# Begin Source File

SOURCE=..\src\watchwindow.h
# End Source File
# Begin Source File

SOURCE=..\src\widelands.h
# End Source File
# Begin Source File

SOURCE=..\src\worker.h
# End Source File
# Begin Source File

SOURCE=..\src\world.h
# End Source File
# Begin Source File

SOURCE=..\src\worlddata.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
