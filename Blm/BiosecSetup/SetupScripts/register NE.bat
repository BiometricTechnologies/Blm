@echo off

xcopy /y "%~dp0msvs\*.dll" "C:\Windows\System32\" 
:: prev version uninstallation
:: Remove IZ Service
echo Removing old version
echo Removing IZService
sc stop "IdentaZone Collector Service"
set DOTNETFX2=%SystemRoot%\Microsoft.NET\Framework\v4.0.30319
set PATH=%PATH%;%DOTNETFX2%
C:\Windows\Microsoft.NET\Framework\v4.0.30319\InstallUtil /u C:\IdentaZone\IZService.exe
:: Unregister ExplorerDataProvider
echo Removing explorerdataprovider.dll
regsvr32.exe /u "%~dp0explorerdataprovider.dll"
echo removing server
del "c:\IdentaZone\BioControls.dll"
del "c:\IdentaZone\CollectorDialog.exe"
del "c:\IdentaZone\CollectorServices.dll"
del "c:\IdentaZone\IMPlugin.dll"
del "c:\IdentaZone\IZService.exe"
del "c:\IdentaZone\log4net.dll"
del "c:\IdentaZone\PluginManager.dll"
del "c:\IdentaZone\SecuBSPMx.NET.dll"
del "c:\IdentaZone\SecuGen.FDxSDKPro.Windows.dll"
del "c:\IdentaZone\Plugins\DP.dll"
del "c:\IdentaZone\Plugins\SG.dll"
del "c:\IdentaZone\Plugins\IB.dll"
echo removing client
del "c:\windows\System32\IdentaZoneAP.dll"
del "c:\windows\CollectorClient.dll"
del "c:\windows\CollectorClient.dll.reg"
del "c:\windows\CollectorServices.dll"
del "c:\windows\log4net.dll"

:: new version installation
xcopy /y "%~dp0\CppPlugins\*.dll" "C:\Windows\System32\IdentaZone\Biosecure\"
regsvr32.exe  "%~dp0explorerdataprovider.dll"
xcopy "%~dp0BioControls" "c:\IdentaZone\" /e /i /h
copy "%~dp0IdentaZoneAP.dll" "c:\windows\system32\"
copy "%~dp0CollectorClient\CollectorClient.dll.reg" "c:\windows\system32\"
xcopy "%~dp0CollectorClient" "c:\windows\" /e /i /h
start CollectorClient.dll.reg
set DOTNETFX2=%SystemRoot%\Microsoft.NET\Framework\v4.0.30319
set PATH=%PATH%;%DOTNETFX2%
%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\InstallUtil C:\IdentaZone\IZService.exe
:: Start windows service
sc start "IdentaZone Collector Service"

pause 