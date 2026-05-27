@echo off
setlocal
cd /d "%~dp0\.."
powershell -NoProfile -ExecutionPolicy Bypass -File "tools\patch_wxdebug_cdisp_clang.ps1"
endlocal
