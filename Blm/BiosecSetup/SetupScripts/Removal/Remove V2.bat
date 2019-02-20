REGEDIT4

regsvr32.exe /u "%~dp0explorerdataprovider.dll"
del "c:\windows\BioControls.dll"
del "c:\windows\Dialog.dll"
del "c:\windows\Dialog.dll.config"
del "c:\windows\Dialog.dll.reg"
del "c:\windows\IMPlugin.dll"
del "c:\windows\log4net.dll"
del "c:\windows\Mocker.exe"
del "c:\windows\PluginManager.dll"
del "c:\windows\SecuBSPMx.NET.dll"
del "c:\windows\SecuGen.FDxSDKPro.Windows.dll"
del "c:\windows\Plugins\DP.dll"
del "c:\windows\Plugins\SG.dll"
del "c:\windows\Plugins\IB.dll"
del "c:\windows\System32\IdentaZoneAP.dll"

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

pause 