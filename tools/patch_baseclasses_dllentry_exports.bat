@echo off
setlocal

cd /d C:\codes\zig\vcam

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p='third_party\baseclasses\dllentry.cpp';" ^
  "$s=[IO.File]::ReadAllText($p);" ^
  "$s=$s -replace '__control_entrypoint\(DllExport\)\s*STDAPI\s+DllGetClassObject\s*\(', 'extern \"C\" __declspec(dllexport) HRESULT __stdcall DllGetClassObject(';" ^
  "$s=$s -replace '(?m)^STDAPI\s+DllCanUnloadNow\s*\(\s*\)', 'extern \"C\" __declspec(dllexport) HRESULT __stdcall DllCanUnloadNow()';" ^
  "[IO.File]::WriteAllText($p,$s);"

if errorlevel 1 (
  echo Patch failed.
  exit /b 1
)

echo Patched third_party\baseclasses\dllentry.cpp exports.
endlocal
