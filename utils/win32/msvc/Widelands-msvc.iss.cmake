; ===================================== ;
; Widelands-Inno-Setup-Script           ;
; ===================================== ;
;                                       ;
; You will need a current version of    ;
; the "Inno Setup Compiler" and the     ;
; "Inno Setup QuickStart Pack" to com-  ;
; pile a Setup with use of this script. ;
;                                       ;
; For more information visit:           ;
; http://www.innosetup.com              ;
;                                       ;
; ===================================== ;


; CHANGES BEFORE COMPILE:
;
; Only change the "Placeholder"-variables in the two "#define"-sections.
; Nothing more must be changed. All visible stuff is defined there.


;Version String
#define Name "Widelands"
#define VerName "Widelands @WL_VERSION@"
#define VerNumber "@WL_VERSION_MAJOR@.@WL_VERSION_MINOR@.@WL_VERSION_PATCH@"
#define Copyright "Widelands Development Team 2001-2014"
#define SetupFileName "Widelands-@WL_VERSION@-win32"

;General String
#define Publisher "Widelands Development Team"
#define URL "http://www.widelands.org"
#define HelpURL "http://wl.widelands.org/wiki/MainPage/"
#define ExeName "widelands.exe"
#define UrlName "Widelands.url"
#define HelpName "Widelands-Onlinehelp.url"
#define HelpNameName "Widelands-Onlinehelp"
#define Copying "COPYING.txt"

[Setup]
AppName={#Name}
AppVerName={#VerName}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
AppUpdatesURL={#URL}
DefaultDirName={pf}\{#Name}
DefaultGroupName={#Name}
AllowNoIcons=true
LicenseFile=@CMAKE_CURRENT_SOURCE_DIR@\..\COPYING
InfoAfterFile=@CMAKE_CURRENT_SOURCE_DIR@\..\ChangeLog
OutputDir=@CMAKE_CURRENT_BINARY_DIR@\..\
OutputBaseFilename={#SetupFileName}
SetupIconFile=@CMAKE_CURRENT_SOURCE_DIR@\..\build\win32\WL.ico
Compression=lzma/ultra
SolidCompression=true
VersionInfoVersion={#VerNumber}
VersionInfoCompany={#Publisher}
VersionInfoDescription={#VerName} Setup
ShowLanguageDialog=yes
WizardImageFile=@CMAKE_CURRENT_SOURCE_DIR@\..\utils\win32\innosetup\WL.bmp
WizardSmallImageFile=@CMAKE_CURRENT_SOURCE_DIR@\utils\build\win32\innosetup\WLsmall.bmp
AppVersion={#VerName}
UninstallDisplayIcon={app}\unins000.exe
UninstallDisplayName={#VerName}
VersionInfoCopyright={#Publisher}
InternalCompressLevel=max
AppID={{WIDELANDS-WIN32-IS}
AppCopyright={#Copyright}
ChangesAssociations=yes

[Languages]
Name: english;   MessagesFile: compiler:Default.isl
Name: czech;     MessagesFile: compiler:Languages\Czech.isl
Name: finnish;   MessagesFile: compiler:Languages\Finnish.isl
Name: french;    MessagesFile: compiler:Languages\French.isl
Name: german;    MessagesFile: compiler:Languages\German.isl
Name: polish;    MessagesFile: compiler:Languages\Polish.isl
Name: swedish;   MessagesFile: compiler:Languages\Swedish.isl
Name: slovak;    MessagesFile: compiler:Languages\Slovak.isl
Name: russian;   MessagesFile: compiler:Languages\Russian.isl
Name: hungarian; MessagesFile: compiler:Languages\Hungarian.isl
Name: dutch;     MessagesFile: compiler:Languages\Dutch.isl
Name: gaelic;    MessagesFile: compiler:Languages\ScottishGaelic.isl
Name: hebrew;    MessagesFile: compiler:Languages\Hebrew.isl
Name: russian;   MessagesFile: compiler:Languages\Russian.isl
Name: spanish;   MessagesFile: compiler:Languages\Spanish.isl

[Tasks]
Name: desktopicon;     Description: {cm:CreateDesktopIcon};     GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\campaigns\*;  DestDir: {app}\campaigns\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\fonts\*;      DestDir: {app}\fonts\;     Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_BINARY_DIR@\..\locale\*;     DestDir: {app}\locale\;    Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\maps\*;       DestDir: {app}\maps\;      Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ;                  Components: " Maps"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\music\*;      DestDir: {app}\music\;     Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ;                  Components: " Music"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\pics\*;       DestDir: {app}\pics\;      Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\sound\*;      DestDir: {app}\sound\;     Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Sound"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\tribes\*;     DestDir: {app}\tribes\;    Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\txts\*;       DestDir: {app}\txts\;      Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\worlds\*;     DestDir: {app}\worlds\;    Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\global\*;     DestDir: {app}\global\;    Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\scripting\*;  DestDir: {app}\scripting\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: @CMAKE_CURRENT_BINARY_DIR@\@CMAKE_BUILD_TYPE@\widelands.exe;           DestDir: {app};            Flags: ignoreversion;                                                       Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\ChangeLog;                               DestDir: {app};            Flags: ignoreversion; DestName: ChangeLog.txt;                              Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\COPYING;                                 DestDir: {app};            Flags: ignoreversion; DestName: COPYING.txt;                                Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\CREDITS;                                 DestDir: {app};            Flags: ignoreversion; DestName: CREDITS.txt;                                Components: " Widelands"
Source: @CMAKE_CURRENT_SOURCE_DIR@\..\utils\win32\innosetup\WL-Editor.ico;     DestDir: {app};            Flags: ignoreversion;                                                       Components: " Widelands"

[INI]
Filename: {app}\{#UrlName};  Section: InternetShortcut; Key: URL; String: {#URL}
Filename: {app}\{#HelpName}; Section: InternetShortcut; Key: URL; String: {#HelpURL}

[Icons]
Name: {app}\{#Name} - Mapeditor;             Filename: {app}\{#ExeName}; Parameters: " --editor"; WorkingDir: {app}; IconFilename: {app}\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths
Name: {group}\{#Name};                       Filename: {app}\{#ExeName}; WorkingDir: {app}; IconFilename: {app}\widelands.exe; Flags: useapppaths; Tasks: ; Languages:
Name: {group}\{cm:ProgramOnTheWeb,{#Name}};  Filename: {app}\{#UrlName}
Name: {group}\{cm:UninstallProgram,{#Name}}; Filename: {uninstallexe}
Name: {group}\{#HelpNameName};               Filename: {app}\{#HelpName}; Tasks: ; Languages:
Name: {group}\{#Name} - Mapeditor;           Filename: {app}\{#ExeName}; Parameters: " --editor"; WorkingDir: {app}; IconFilename: {app}\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#Name}; Filename: {app}\{#ExeName}; Tasks: quicklaunchicon; WorkingDir: {app}; Flags: useapppaths; IconFilename: {app}\widelands.exe
Name: {userdesktop}\{#Name};                 Filename: {app}\{#ExeName}; Tasks: desktopicon; WorkingDir: {app}; Flags: useapppaths; IconFilename: {app}\widelands.exe
Name: {userdesktop}\{#Name} - Mapeditor;     Filename: {app}\{#ExeName}; Parameters: " --editor"; Tasks: desktopicon; WorkingDir: {app}; IconFilename: {app}\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths
Name: {group}\{#Copying};                    Filename: {app}\{#Copying}

[Run]
Filename: {app}\{#ExeName}; Description: {cm:LaunchProgram,{#Name}}; Flags: nowait postinstall skipifsilent

[InstallDelete]
Type: filesandordirs; Name: {app}\campaigns\*
Type: filesandordirs; Name: {app}\global\*
Type: filesandordirs; Name: {app}\locale\*
Type: filesandordirs; Name: {app}\maps\*
Type: filesandordirs; Name: {app}\pics\*
Type: filesandordirs; Name: {app}\scripting\*
Type: filesandordirs; Name: {app}\sound\*
Type: filesandordirs; Name: {app}\tribes\*
Type: filesandordirs; Name: {app}\txts\*
Type: filesandordirs; Name: {app}\worlds\*

[UninstallDelete]
Type: files; Name: {app}\{#UrlName}
Type: files; Name: {app}\{#HelpName}
Type: files; Name: {app}\stdout.txt
Type: files; Name: {app}\stderr.txt

[Components]
Name: Widelands; Description: Widelands Core;             Flags: fixed checkablealone; Types: custom compact full
Name: Music;     Description: Widelands Background Music;                              Types: full
Name: Sound;     Description: Widelands Sound Effects;                                 Types: compact full
Name: Maps;      Description: Widelands Maps;                                          Types: compact full

[Registry]
Root: HKCR; Subkey: .wgf;                                 ValueType: string; ValueName: ; ValueData: WidelandsSavegame;  Flags: uninsdeletevalue
Root: HKCR; Subkey: WidelandsSavegame;                    ValueType: string; ValueName: ; ValueData: Widelands Savegame; Flags: uninsdeletekey
Root: HKCR; Subkey: WidelandsSavegame\DefaultIcon;        ValueType: string; ValueName: ; ValueData: {app}\{#ExeName},0
Root: HKCR; Subkey: WidelandsSavegame\shell\open\command; ValueType: string; ValueName: ; ValueData: """{app}\{#ExeName}"" ""--loadgame=%1"""

Root: HKCR; Subkey: .wmf;                                 ValueType: string; ValueName: ; ValueData: WidelandsMapFile;  Flags: uninsdeletevalue
Root: HKCR; Subkey: WidelandsMapFile;                     ValueType: string; ValueName: ; ValueData: Widelands Mapfile; Flags: uninsdeletekey
Root: HKCR; Subkey: WidelandsMapFile\DefaultIcon;         ValueType: string; ValueName: ; ValueData: {app}\WL-Editor.ico
Root: HKCR; Subkey: WidelandsMapFile\shell\open\command;  ValueType: string; ValueName: ; ValueData: """{app}\{#ExeName}"" ""--editor=%1"""
