; -- Example1.iss --
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=Photo Manager (client)
AppVerName=Photo Manager (client) 1.2.3
AppPublisher=Alex Y. Gronsky
AppVersion=1.2.3
OutputBaseFilename=photomanager-1.2.3-client-setup
OutputDir="..\1.2.arch"

DefaultDirName={pf}\Photo Manager (client)
DefaultGroupName=Photo Manager (client)

InfoBeforeFile=textbefore.txt

[Icons]
Name: "{group}\Photo Manager (client) 1.2.3"; Filename: "{app}\PhotoCatalog.exe"; WorkingDir: "{app}"
Name: "{group}\Uninstall Photo Manager"; Filename: "{uninstallexe}"
Name: "{group}\Manual"; Filename: "{app}\manual.pdf"


[Dirs]
Name: "{app}\lang"
Name: "{app}\sqldrivers"
Name: "{app}\imageformats"
Name: "{app}\cache"


[Files]
Source: "C:\Documents and Settings\alex\My Documents\prog\cpp\PhotoCatalog_files\1.2.x\bin\*"; DestDir: "{sys}"; Flags: sharedfile onlyifdoesntexist uninsneveruninstall
Source: "release\PhotoCatalog.exe"; DestDir: "{app}"; Flags: confirmoverwrite
Source: "C:\Documents and Settings\alex\My Documents\prog\cpp\PhotoCatalog_files\1.2.x\sqldrivers\*"; DestDir: "{app}\sqldrivers";
Source: "C:\Documents and Settings\alex\My Documents\prog\cpp\PhotoCatalog_files\1.2.x\imageformats\*"; DestDir: "{app}\imageformats";
Source: "pm_ru.qm"; DestDir: "{app}\lang";
Source: "C:\Documents and Settings\alex\My Documents\prog\cpp\PhotoCatalog_files\1.2.x\vcredist_x86.exe"; DestDir: "{app}";
Source: "docs\manual\manual.pdf"; DestDir: "{app}"

[Run]
Filename: "{app}\vcredist_x86.exe";

