IF "%IsGitlabRunner%"=="1" EXIT
taskkill /IM "S4_Main.exe" /F > nul 2> nul

SET SPATH="D:\Ubisoft Games\thesettlers4\plugins"
SET SEXE1="D:\Ubisoft Games\thesettlers4\S4_Main.exe"

SET SPATH2="C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\games\thesettlers4\plugins\"
SET SEXE2="C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\games\thesettlers4\S4_Main.exe"

SET IN="%1%"
:: Remove invalid "
for /f "useback tokens=*" %%a in ('%IN%') do set IN=%%~a

if exist %SPATH% (
	echo Copying %IN% to %SPATH%...
	xcopy /Q /Y  %IN% %SPATH%
) else (
	if exist %SPATH2% (
		echo Copying %IN% to %SPATH2%...
		xcopy /Q /Y  %IN% %SPATH2%
	)
)