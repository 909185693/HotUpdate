@echo off

setlocal ENABLEDELAYEDEXPANSION
set P4Server=%~3%
set P4Username=%~4%
set P4Password=%~5%
set P4Workspaces=%~6%
p4 set P4PORT=%P4Server%
p4 set P4USER=%P4Username%
echo %P4Password%|p4 login
p4 set P4CLIENT=%P4Workspaces%
for /f "tokens=2 delims= " %%i in ('p4 changes -m1 #have') do (
	set VersionNumber=%%i
	break
)

echo GAME_MAJOR_VERSION: %~1%
echo GAME_MINOR_VERSION: %~2%
echo GAME_BUILD_VERSION: %VersionNumber%
set GAME_MAJOR_VERSION=GAME_MAJOR_VERSION
set GAME_MINOR_VERSION=GAME_MINOR_VERSION
set GAME_BUILD_VERSION=GAME_BUILD_VERSION
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
	echo %%j| findstr %GAME_BUILD_VERSION% >nul && (
		set BuildVesion=%%j
		for /f "tokens=3 delims= " %%k in ("!BuildVesion!") do (
			set bFound=true
			set "BuildVesion=!BuildVesion:%%k=%VersionNumber%!"
			echo !BuildVesion!
			break
		)
	)
	if !bFound!==false (
		echo/%%j
	)
)>>%Tmpfilename%

del %~dp0%Filename%
move %Tmpfilename% %Filename%