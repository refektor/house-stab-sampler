; --------------------------------------
; Plugin Installer Script for House Stab Samples
; --------------------------------------

#define PluginName "HouseStabSamples"
#define CompanyName "StairwayAudio"
#define Version "1.0.0"
#define BuildDir "..\build\artifacts"

[Setup]
AppName={#PluginName}
AppVersion={#Version}
DefaultDirName={commoncf}\VST3\{#CompanyName}\{#PluginName}
DefaultGroupName={#CompanyName}
UninstallDisplayIcon={app}\{#PluginName}.ico
OutputBaseFilename={#PluginName}Installer
ArchitecturesInstallIn64BitMode=x64

[Files]
; VST3 plugin goes to the common VST3 folder
Source: "{#BuildDir}\VST3\{#PluginName}.vst3"; DestDir: "{commoncf}\VST3"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\Uninstall {#PluginName}"; Filename: "{uninstallexe}"

[UninstallDelete]
; Optionally clean up extra folders or settings
; Type: dir|file; Name: "full\path"
; Example: Type: dir; Name: "{app}\Presets"