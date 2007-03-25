; Only change the variables below. Nothing more should be changed.

;Version String
#define XAppName "Widelands"
#define XAppVerName "Widelands SVNbuild"
#define XAppVerNumber "0.10.0.1"
#define XAppID "{WIDELAND-ddmm-yyyy-SVN-INOFFICIAL}"

;General String
#define XAppPublisher "Widelands Development Team"
#define XAppURL "http://www.widelands.org"
#define HelpURL "http://xoops.widelands.org/modules/mediawiki/wiki/Help"
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
OutputBaseFilename=Widelands-svn_Setup
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
AppCopyright=Widelands Development Team 2001-2006

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
Source: ..\..\*; DestDir: {app}; Flags: recursesubdirs ignoreversion


[INI]
Filename: {app}\{#XAppUrlName}; Section: InternetShortcut; Key: URL; String: {#XAppURL}
Filename: {app}\{#HelpName}; Section: InternetShortcut; Key: URL; String: {#HelpURL}

[Icons]
Name: {group}\{#XAppName}; Filename: {app}\{#XAppExeName}; WorkingDir: {app}; IconFilename: {app}\widelands.exe; Flags: useapppaths; Tasks: ; Languages: 
Name: {group}\{cm:ProgramOnTheWeb,{#XAppName}}; Filename: {app}\{#XAppUrlName}
Name: {group}\{cm:UninstallProgram,{#XAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#XAppName}; Filename: {app}\{#XAppExeName}; Tasks: desktopicon; WorkingDir: {app}; Flags: useapppaths; IconFilename: {app}\widelands.exe
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#XAppName}; Filename: {app}\{#XAppExeName}; Tasks: quicklaunchicon; WorkingDir: {app}; Flags: useapppaths; IconFilename: {app}\widelands.exe
Name: {group}\{#HelpNameName}; Filename: {app}\{#HelpName}; Tasks: ; Languages: 
Name: {group}\{#XAppName} - Mapeditor; Filename: {app}\{#XAppExeName}; Parameters: " --editor"; WorkingDir: {app}; IconFilename: {app}\pics\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths
Name: {userdesktop}\{#XAppName} - Mapeditor; Filename: {app}\{#XAppExeName}; Parameters: " --editor"; WorkingDir: {app}; IconFilename: {app}\pics\WL-Editor.ico; Comment: Directly starts the Widelands-Editor; Flags: useapppaths

[Run]
Filename: {app}\{#XAppExeName}; Description: {cm:LaunchProgram,{#XAppName}}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\{#XAppUrlName}
Type: files; Name: {app}\{#HelpName}
