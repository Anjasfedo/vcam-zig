@echo off
setlocal

cd /d "%~dp0\.."

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p='src\VirtualCam.cpp';" ^
  "$s=Get-Content $p -Raw;" ^
  "$s=$s -replace '\*ppmt = CreateMediaType\(&mt\);`r`n\s*return \*ppmt \? S_OK : E_OUTOFMEMORY;', \"*ppmt = CreateMediaType(&mt);`r`n    return *ppmt ? S_OK : E_OUTOFMEMORY;\";" ^
  "$s=$s -replace '\*ppmt = CreateMediaType\(&mt\);`r`n\s*hr = \*ppmt \? S_OK : E_OUTOFMEMORY;', \"*ppmt = CreateMediaType(&mt);`r`n    hr = *ppmt ? S_OK : E_OUTOFMEMORY;\";" ^
  "Set-Content $p $s -Encoding ASCII;"

echo Fixed accidental literal `r`n text in src\VirtualCam.cpp
endlocal
