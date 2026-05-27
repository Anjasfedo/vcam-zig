@echo off
setlocal
cd /d "%~dp0\.."
powershell -NoProfile -ExecutionPolicy Bypass -File "tools\fix_virtualcam_bad_newline_safe.ps1"
endlocal
