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
; You don't even need to clean up your widelands-svn-checkout-directory.
; ".svn"-directorys will be excluded in the compilation-processe.


;Version String
#define XAppName "Widelands"
#define XAppVerName "Widelands Build11"
#define XAppVerNumber "0.11.0.1"
#define XAppID "{WIDELAND-BUILD11-2007}"

;General String
#define XAppPublisher "Widelands Development Team"
#define XAppURL "http://www.widelands.org"
#define HelpURL "http://xoops.widelands.org/modules/mediawiki/wiki/Main_Page"
#define XAppExeName "widelands.exe"
#define XAppUrlName "Widelands.url"
#define HelpName "Widelands-Onlinehelp.url"
#define HelpNameName "Widelands-Onlinehelp"

[Setup]
AppName={#XAppName}
AppVerName={#XAppVerName}
AppPublisher={#XAppPublisher}
AppPublisherURL={#XAppURL}
AppSupportURL={#XAppURL}
AppUpdatesURL={#XAppURL}
DefaultDirName={pf}\{#XAppName}
DefaultGroupName={#XAppName}
AllowNoIcons=true
LicenseFile=..\..\COPYING
InfoAfterFile=..\..\ChangeLog
OutputDir=..\..\..\
OutputBaseFilename=Widelands-SVN-Setup
SetupIconFile=.\WL.ico
Compression=lzma/ultra
SolidCompression=true
VersionInfoVersion={#XAppVerNumber}
VersionInfoCompany={#XAppPublisher}
VersionInfoDescription={#XAppVerName} Setup
ShowLanguageDialog=yes
WizardImageFile=.\innosetup-files\WL.bmp
WizardSmallImageFile=.\innosetup-files\WLsmall.bmp
AppVersion={#XAppVerName}
UninstallDisplayIcon={app}\unins000.exe
UninstallDisplayName={#XAppVerName}
VersionInfoCopyright={#XAppPublisher}
InternalCompressLevel=max
AppID={{#XAppID}
AppCopyright=Widelands Development Team 2001-2007

[Languages]
Name: english; MessagesFile: compiler:Default.isl
Name: czech; MessagesFile: compiler:Languages\Czech.isl
Name: finnish; MessagesFile: compiler:Languages\Finnish.isl
Name: french; MessagesFile: compiler:Languages\French.isl
Name: german; MessagesFile: compiler:Languages\German.isl
Name: polish; MessagesFile: compiler:Languages\Polish.isl
Name: swedish; MessagesFile: compiler:Languages\Swedish.isl
Name: slovak; MessagesFile: compiler:Languages\Slovak.isl
Name: russian; MessagesFile: compiler:Languages\Russian.isl
Name: hungarian; MessagesFile: compiler:Languages\Hungarian.isl
Name: dutch; MessagesFile: compiler:Languages\Dutch.isl
Name: galician; MessagesFile: compiler:Languages\Galician.isl
Name: hebrew; MessagesFile: compiler:Languages\Hebrew.isl
Name: russian; MessagesFile: compiler:Languages\Russian.isl
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: ..\..\campaigns\*; DestDir: {app}\campaigns\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\fonts\*; DestDir: {app}\fonts\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\game_server\*; DestDir: {app}\game_server\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\locale\*; DestDir: {app}\locale\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\maps\*; DestDir: {app}\maps\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Components: " Maps"
Source: ..\..\music\*; DestDir: {app}\music\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Components: " Music"
Source: ..\..\pics\*; DestDir: {app}\pics\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\sound\*; DestDir: {app}\sound\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\tribes\*; DestDir: {app}\tribes\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\txts\*; DestDir: {app}\txts\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\worlds\*; DestDir: {app}\worlds\; Flags: recursesubdirs ignoreversion; Tasks: ; Languages: ; Attribs: hidden; Components: " Widelands"
Source: ..\..\*.dll; DestDir: {app}; Flags: ignoreversion; Components: " Widelands"
Source: ..\..\widelands.exe; DestDir: {app}; Flags: ignoreversion; Components: " Widelands"
Source: ..\..\config; DestDir: {app}; Flags: ignoreversion; Components: " Widelands"
Source: ..\..\ChangeLog; DestDir: {app}; Flags: ignoreversion; DestName: ChangeLog.txt; Components: " Widelands"
Source: ..\..\COPYING; DestDir: {app}; Flags: ignoreversion; DestName: COPYING.txt; Components: " Widelands"
Source: ..\..\CREDITS; DestDir: {app}; Flags: ignoreversion; DestName: CREDITS.txt; Components: " Widelands"
Source: .\innosetup-files\WL-Editor.ico; DestDir: {app}; Flags: ignoreversion; Components: " Widelands"


[INI]
Filename: {app}\{#XAppUrlName}; Section: InternetShortcut; Key: URL; String: {#XAppURL}
Filename: {app}\{#HelpName}; Section: InternetShortcut; Key: URL; String: {#HelpURL}

[Icons]
Name: {app}\{#XAppName} - Mapeditor; Filename: {app}\{#XAppExeName}; Parameters: " --editor"; WorkingDir: {app}; IconFilename: {app}\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths
Name: {group}\{#XAppName}; Filename: {app}\{#XAppExeName}; WorkingDir: {app}; IconFilename: {app}\widelands.exe; Flags: useapppaths; Tasks: ; Languages:
Name: {group}\{cm:ProgramOnTheWeb,{#XAppName}}; Filename: {app}\{#XAppUrlName}
Name: {group}\{cm:UninstallProgram,{#XAppName}}; Filename: {uninstallexe}
Name: {group}\{#HelpNameName}; Filename: {app}\{#HelpName}; Tasks: ; Languages:
Name: {group}\{#XAppName} - Mapeditor; Filename: {app}\{#XAppExeName}; Parameters: " --editor"; WorkingDir: {app}; IconFilename: {app}\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#XAppName}; Filename: {app}\{#XAppExeName}; Tasks: quicklaunchicon; WorkingDir: {app}; Flags: useapppaths; IconFilename: {app}\widelands.exe
Name: {userdesktop}\{#XAppName}; Filename: {app}\{#XAppExeName}; Tasks: desktopicon; WorkingDir: {app}; Flags: useapppaths; IconFilename: {app}\widelands.exe
Name: {userdesktop}\{#XAppName} - Mapeditor; Filename: {app}\{#XAppExeName}; Parameters: " --editor"; Tasks: desktopicon; WorkingDir: {app}; IconFilename: {app}\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths

[Run]
Filename: {app}\{#XAppExeName}; Description: {cm:LaunchProgram,{#XAppName}}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\{#XAppUrlName}
Type: files; Name: {app}\{#HelpName}
Type: files; Name: {app}\stdout.txt
Type: files; Name: {app}\stderr.txt

[Components]
Name: Widelands; Description: Widelands Core; Flags: fixed checkablealone; Types: custom compact full
Name: Music; Description: Widelands Background Music; Types: full
Name: Maps; Description: Widelands Maps; Types: compact full
