IF "%IsGitlabRunner%"=="1" EXIT
taskkill /IM "S4_Main.exe" /F > nul 2> nul

SET SPATH="I:\Games\Siedler\thesettlers4\plugins\"
SET SEXE1="I:\Games\Siedler\thesettlers4\S4_Main.exe"

SET SPATH2="C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\games\thesettlers4\plugins\"
SET SEXE2="C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\games\thesettlers4\S4_Main.exe"

SET IN="%1%"
:: Remove invalid "
for /f "useback tokens=*" %%a in ('%IN%') do set IN=%%~a

if exist %SPATH% (
	echo Copying %IN% to %SPATH%...
	xcopy /Q /Y  %IN% %SPATH%
	start "" SEXE1%
) else (
	if exist %SPATH2% (
		echo Copying %IN% to %SPATH2%...
		xcopy /Q /Y  %IN% %SPATH2%
		start "" %SEXE2%
	)
)