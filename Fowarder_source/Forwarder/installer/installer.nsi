
Name "Forwarder Service"

; The file to write
OutFile "ForwarderInstaller.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Forwarder Service"

; The text to prompt the user to enter a directory
DirText "Choose install directory"

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

; Set output path to the installation directory.
SetOutPath $INSTDIR

; Put file there
File ..\release\Forwarder.exe
File ..\config.xml
File ..\license.txt
File ..\version.txt
nsSCM::Install fwdsrv "Forwarder Service" 16 2 "$INSTDIR\Forwarder.exe" "" "" "" ""
WriteRegStr HKLM "SOFTWARE\TommiRouvali\Forwarder" "ConfigFile" "$INSTDIR\config.xml"
WriteRegStr HKLM "SOFTWARE\TommiRouvali\Forwarder" "logFile" "$INSTDIR\log.txt"
writeUninstaller $INSTDIR\uninstaller.exe
Exec 'notepad "$INSTDIR\config.xml"'
SectionEnd

Section "Uninstall"
nsSCM::Stop fwdsrv
nsSCM::Remove fwdsrv 
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\TommiRouvali\Forwarder"
Delete $INSTDIR\Forwarder.exe
Delete $INSTDIR\config.xml
Delete $INSTDIR\log.txt
RmDir $INSTDIR

SectionEnd