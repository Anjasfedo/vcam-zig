@echo off
setlocal

cd /d C:\codes\zig\vcam

if not exist zig-out\bin mkdir zig-out\bin

zig c++ ^
  -target x86_64-windows-msvc ^
  -shared ^
  -fno-exceptions ^
  -fno-rtti ^
  -nostdlib++ ^
  -o zig-out\bin\ZigVirtualCam.ax ^
  src\dllmain.cpp ^
  src\VirtualCam.cpp ^
  third_party\baseclasses\amfilter.cpp ^
  third_party\baseclasses\amvideo.cpp ^
  third_party\baseclasses\arithutil.cpp ^
  third_party\baseclasses\combase.cpp ^
  third_party\baseclasses\cprop.cpp ^
  third_party\baseclasses\ctlutil.cpp ^
  third_party\baseclasses\ddmm.cpp ^
  third_party\baseclasses\dllentry.cpp ^
  third_party\baseclasses\dllsetup.cpp ^
  third_party\baseclasses\mtype.cpp ^
  third_party\baseclasses\outputq.cpp ^
  third_party\baseclasses\pstream.cpp ^
  third_party\baseclasses\pullpin.cpp ^
  third_party\baseclasses\renbase.cpp ^
  third_party\baseclasses\source.cpp ^
  third_party\baseclasses\strmctl.cpp ^
  third_party\baseclasses\transfrm.cpp ^
  third_party\baseclasses\videoctl.cpp ^
  third_party\baseclasses\vtrans.cpp ^
  third_party\baseclasses\winctrl.cpp ^
  third_party\baseclasses\winutil.cpp ^
  third_party\baseclasses\wxdebug.cpp ^
  third_party\baseclasses\wxlist.cpp ^
  third_party\baseclasses\wxutil.cpp ^
  -Isrc ^
  -Ithird_party\baseclasses ^
  -DUNICODE ^
  -D_UNICODE ^
  -DWIN32_LEAN_AND_MEAN ^
  -D_CRT_SECURE_NO_WARNINGS ^
  -Wno-microsoft-exception-spec ^
  -Wno-writable-strings ^
  -Wno-nonportable-include-path ^
  -Wno-microsoft-extra-qualification ^
  -lole32 ^
  -loleaut32 ^
  -lstrmiids ^
  -luuid ^
  -lwinmm ^
  -luser32 ^
  -lgdi32 ^
  -ladvapi32 ^
  -lddraw ^
  -ldxguid

if errorlevel 1 (
  echo.
  echo BUILD FAILED with exit code %ERRORLEVEL%
  exit /b %ERRORLEVEL%
)

if not exist zig-out\bin\ZigVirtualCam.ax (
  echo.
  echo BUILD COMMAND FINISHED, BUT .AX FILE WAS NOT CREATED
  exit /b 1
)

echo.
echo BUILD OK:
echo C:\codes\zig\vcam\zig-out\bin\ZigVirtualCam.ax

endlocal
