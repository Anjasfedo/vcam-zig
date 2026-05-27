@echo off
setlocal EnableExtensions

rem ============================================================
rem complete_clean.bat
rem
rem Cleans generated Zig build artifacts for the DirectShow
rem virtual camera project.
rem
rem Safe to run from anywhere. It resolves project root as:
rem   tools\..\
rem
rem It deletes:
rem   .zig-cache
rem   zig-out
rem   build logs commonly created during this journey
rem
rem It does NOT delete:
rem   src
rem   third_party\baseclasses
rem   build_manual_x64_msvc.bat
rem   JOURNEY.md
rem ============================================================

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."

pushd "%PROJECT_ROOT%" >nul 2>nul
if errorlevel 1 (
    echo Failed to enter project root:
    echo   %PROJECT_ROOT%
    exit /b 1
)

echo.
echo [complete_clean] Project root:
cd
echo.

rem Optional safety info: warn if output AX may still be registered.
if exist "zig-out\bin\ZigVirtualCam.ax" (
    echo [complete_clean] Existing build output found:
    echo   zig-out\bin\ZigVirtualCam.ax
    echo.
    echo If this file is currently loaded by an app, deletion may fail.
    echo Close OBS, Chrome, Meet, GraphStudioNext, VLC, etc. before cleaning.
    echo.
)

echo [complete_clean] Removing Zig cache/output folders...

if exist ".zig-cache" (
    rmdir /S /Q ".zig-cache"
    if exist ".zig-cache" (
        echo Failed to remove .zig-cache
        popd >nul
        exit /b 1
    )
    echo   removed .zig-cache
) else (
    echo   .zig-cache not present
)

if exist "zig-out" (
    rmdir /S /Q "zig-out"
    if exist "zig-out" (
        echo Failed to remove zig-out
        popd >nul
        exit /b 1
    )
    echo   removed zig-out
) else (
    echo   zig-out not present
)

echo.
echo [complete_clean] Removing common generated logs/temp files...

for %%F in (
    build_log.txt
    build.log
    compile.log
    link.log
    zig_build.log
    regsvr32.log
) do (
    if exist "%%F" (
        del /F /Q "%%F" >nul 2>nul
        echo   removed %%F
    )
)

rem Recreate output folder so build scripts have a predictable place.
mkdir "zig-out\bin" >nul 2>nul

echo.
echo [complete_clean] Clean complete.
echo.
echo Next step:
echo   build_manual_x64_msvc.bat
echo.

popd >nul
endlocal
