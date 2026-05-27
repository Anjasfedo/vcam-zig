@echo off
setlocal

cd /d %~dp0\..

if not exist src\VirtualCam.cpp (
  echo ERROR: src\VirtualCam.cpp not found. Extract this ZIP into your vcam project root.
  exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p='src\\VirtualCam.cpp';" ^
  "$s=Get-Content $p -Raw;" ^
  "$s=$s -replace 'return CopyMediaType\(ppmt, &mt\);', '*ppmt = CreateMediaType(&mt);`r`n    return *ppmt ? S_OK : E_OUTOFMEMORY;';" ^
  "$s=$s -replace 'hr = CopyMediaType\(ppmt, &mt\);', '*ppmt = CreateMediaType(&mt);`r`n    hr = *ppmt ? S_OK : E_OUTOFMEMORY;';" ^
  "Set-Content $p $s -NoNewline;" 

if errorlevel 1 (
  echo ERROR: patch failed.
  exit /b 1
)

echo Patched src\VirtualCam.cpp CopyMediaType usage for DirectShow BaseClasses.
echo Now run: build_manual_x64_msvc.bat
endlocal
