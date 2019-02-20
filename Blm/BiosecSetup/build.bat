@echo off
:: BioSec
copy ..\biosec\Release\ExplorerDataProvider.dll ..\Deploy\
copy ..\Release\IdentaZoneAP.dll ..\Deploy\
:: msvs
xcopy /y libs\msvs\*.dll ..\Deploy\msvs\
:: Plugin DLLs
xcopy /y ..\Release_x86\*.dll ..\Deploy\CppPlugins\
:: Collector GUI + Server
xcopy /y ..\BioCollector\IZService\bin\Release\*.dll ..\Deploy\BioControls\
xcopy /y ..\BioCollector\IZService\bin\Release\*.exe ..\Deploy\BioControls\
xcopy /y libs\* ..\Deploy\BioControls\
xcopy /y libs\Plugins\* ..\Deploy\BioControls\Plugins\
:: Collector Client
xcopy /y ..\BioCollector\CollectorClient\bin\Release\*.dll* ..\Deploy\CollectorClient\
:: Add Setup Script
copy "SetupScripts\register NE.bat" ..\Deploy\Setup.bat
copy "SetupScripts\RemoveAssociations.bat" ..\Deploy\RemoveAssociations.bat
copy "SetupScripts\Remove V1.bat" ..\Deploy\UninstallFirstVersion.bat

pause
