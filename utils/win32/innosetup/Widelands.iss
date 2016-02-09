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
#define VerName "Widelands Build19"
#define VerNumber "0.19.0.1"
#define Copyright "Widelands Development Team 2001-2015"
#define SetupFileName "Widelands-Build19-win64"

;General String
#define Publisher "Widelands Development Team"
#define URL "http://www.widelands.org"
#define HelpURL "http://wl.widelands.org/wiki/MainPage/"
#define ExeName "widelands.exe"
#define UrlName "Widelands.url"
#define HelpName "Widelands-Onlinehelp.url"
#define HelpNameName "Widelands-Onlinehelp"
#define Copying "COPYING.txt"

;Appveyor environment
#define BuildFolder GetEnv("APPVEYOR_BUILD_FOLDER")
#define DLLFolder GetEnv("MINGWPATH")

[Setup]
AppName={#Name}
AppVerName={#VerName}
AppVersion={#VerNumber}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
AppUpdatesURL={#URL}
DefaultDirName={pf}\{#Name}
DefaultGroupName={#Name}
AllowNoIcons=true
LicenseFile=..\..\..\COPYING
InfoAfterFile=..\..\..\ChangeLog
OutputDir=..\..\..\..\
OutputBaseFilename={#SetupFileName}
SetupIconFile=..\WL.ico
Compression=lzma/ultra
SolidCompression=true
VersionInfoCompany={#Publisher}
VersionInfoDescription={#VerName} Setup
ShowLanguageDialog=yes
WizardImageFile=.\WL.bmp
WizardSmallImageFile=.\WLsmall.bmp
UninstallDisplayIcon={app}\unins000.exe
UninstallDisplayName={#VerName}
VersionInfoCopyright={#Publisher}
InternalCompressLevel=max
AppID={{WIDELANDS-WIN32-IS}
AppCopyright={#Copyright}
ChangesAssociations=yes
PrivilegesRequired=lowest

[Languages]
Name: english;   MessagesFile: compiler:Default.isl
Name: czech;     MessagesFile: compiler:Languages\Czech.isl
Name: finnish;   MessagesFile: compiler:Languages\Finnish.isl
Name: french;    MessagesFile: compiler:Languages\French.isl
Name: german;    MessagesFile: compiler:Languages\German.isl
Name: polish;    MessagesFile: compiler:Languages\Polish.isl
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
Source: {#BuildFolder}\data\campaigns\*;                               DestDir: {app}\data\campaigns\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\i18n\*;                                    DestDir: {app}\data\i18n\;      Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\build\locale\*;                                 DestDir: {app}\data\locale\;    Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\maps\*;                                    DestDir: {app}\data\maps\;      Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ;                  Components: "Widelands"
Source: {#BuildFolder}\data\music\*;                                   DestDir: {app}\data\music\;     Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ;                  Components: "Music"
Source: {#BuildFolder}\data\images\*;                                  DestDir: {app}\data\images\;      Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\sound\*;                                   DestDir: {app}\data\sound\;     Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\tribes\*;                                  DestDir: {app}\data\tribes\;    Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\txts\*;                                    DestDir: {app}\data\txts\;      Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\world\*;                                   DestDir: {app}\data\world\;     Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\scripting\*;                               DestDir: {app}\data\scripting\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#BuildFolder}\data\shaders\*;                                 DestDir: {app}\data\shaders\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: "Widelands"
Source: {#DLLFolder}\libglbinding.dll;                        DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libicuuc56.dll;                    DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libbz2-1.dll;                      DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libfreetype-6.dll;                 DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
#if GetEnv("PLATFORM") == "x64"
Source: {#DLLFolder}\libgcc_s_seh-1.dll;                DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
#else
Source: {#DLLFolder}\libgcc_s_dw2-1.dll;                DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
#endif
Source: {#DLLFolder}\libglib-2.0-0.dll;                 DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libharfbuzz-0.dll;                 DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libicudt56.dll;                    DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libjpeg-8.dll;                     DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\liblzma-5.dll;                     DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libogg-0.dll;                      DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libpng16-16.dll;                   DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libstdc++-6.dll;              DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libtiff-5.dll;                DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libvorbis-0.dll;              DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libvorbisfile-3.dll;          DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libwebp-6.dll;                DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libwinpthread-1.dll;          DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\SDL2.dll;                     DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\SDL2_image.dll;               DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\SDL2_mixer.dll;               DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\SDL2_net.dll;                 DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\SDL2_ttf.dll;                 DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\zlib1.dll;                    DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libFLAC-8.dll;                DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libfluidsynth-1.dll;          DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libportaudio-2.dll;           DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libsndfile-1.dll;             DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libspeex-1.dll;               DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libvorbisenc-2.dll;           DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libmodplug-1.dll;             DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libmad-0.dll;                 DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libintl-8.dll;                         DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#DLLFolder}\libiconv-2.dll;                        DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#BuildFolder}\build\src\widelands.exe;   DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"
Source: {#BuildFolder}\ChangeLog;                                 DestDir: {app};            Flags: ignoreversion; DestName: ChangeLog.txt;                              Components: "Widelands"
Source: {#BuildFolder}\COPYING;                                   DestDir: {app};            Flags: ignoreversion; DestName: COPYING.txt;                                Components: "Widelands"
Source: {#BuildFolder}\CREDITS;                                   DestDir: {app};            Flags: ignoreversion; DestName: CREDITS.txt;                                Components: "Widelands"
Source: WL-Editor.ico;                                    DestDir: {app};            Flags: ignoreversion;                                                       Components: "Widelands"

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
Type: filesandordirs; Name: {app}\data\*

[UninstallDelete]
Type: files; Name: {app}\{#UrlName}
Type: files; Name: {app}\{#HelpName}

[Components]
Name: Widelands; Description: Widelands Core;             Flags: fixed checkablealone; Types: custom compact full
Name: Music;     Description: Widelands Background Music;                              Types: full

[Registry]
Root: HKCU; Subkey: Software\Classes\.wgf;                                 ValueType: string; ValueName: ; ValueData: WidelandsSavegame;  Flags: uninsdeletevalue
Root: HKCU; Subkey: Software\Classes\WidelandsSavegame;                    ValueType: string; ValueName: ; ValueData: Widelands Savegame; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\Classes\WidelandsSavegame\DefaultIcon;        ValueType: string; ValueName: ; ValueData: {app}\{#ExeName},0
Root: HKCU; Subkey: Software\Classes\WidelandsSavegame\shell\open\command; ValueType: string; ValueName: ; ValueData: """{app}\{#ExeName}"" ""--loadgame=%1"""

Root: HKCU; Subkey: Software\Classes\.wmf;                                 ValueType: string; ValueName: ; ValueData: WidelandsMapFile;  Flags: uninsdeletevalue
Root: HKCU; Subkey: Software\Classes\WidelandsMapFile;                     ValueType: string; ValueName: ; ValueData: Widelands Mapfile; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\Classes\WidelandsMapFile\DefaultIcon;         ValueType: string; ValueName: ; ValueData: {app}\WL-Editor.ico
Root: HKCU; Subkey: Software\Classes\WidelandsMapFile\shell\open\command;  ValueType: string; ValueName: ; ValueData: """{app}\{#ExeName}"" ""--editor=%1"""
