echo off

cd ../../../
echo %cd%
subwcrev.exe ./ "./Version.config" "./Studio/HotUpdate/Source/HotUpdate/Public/Version.h"

set GAME_MAJOR_VERSION=GAME_MAJOR_VERSION
set GAME_MINOR_VERSION=GAME_MINOR_VERSION
set Tmpfilename=Version.tmp
set Filename=..\..\Source\HotUpdate\Public\Version.h
for /f "tokens=1* delims=:" %%i in ('findstr /n .* %Filename%') do (
	set bFound=false
	echo %%j| findstr %GAME_MAJOR_VERSION% >nul && (
		set MajorVersion=%%j
		for /f "tokens=3 delims= " %%k in ("!MajorVersion!") do (
			set bFound=true
			set "MajorVersion=!MajorVersion:%%k=%~1%!"
			echo !MajorVersion!
			break
		)
	)
	echo %%j| findstr %GAME_MINOR_VERSION% >nul && (
		set MinorVersion=%%j
		for /f "tokens=3 delims= " %%k in ("!MinorVersion!") do (
			set bFound=true
			set "MinorVersion=!MinorVersion:%%k=%~2%!"
			echo !MinorVersion!
			break
		)
	)
	if !bFound!==false (
		echo/%%j
	)
)>>%Tmpfilename%

del %~dp0%Filename%
move %Tmpfilename% %Filename%
