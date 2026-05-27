# Zig Virtual Camera Journey

This document records the step-by-step journey we followed to create and build a Windows DirectShow virtual camera using C++ source code and Zig as the compiler.

Project root used during this journey:

```text
C:\codes\zig\vcam
```

Final built output:

```text
C:\codes\zig\vcam\zig-out\bin\ZigVirtualCam.ax
```

---

## 1. Goal

The goal was to create a Windows virtual camera using the older DirectShow `.ax` approach.

A `.ax` file is basically a Windows DLL COM server registered as a DirectShow filter. In our case, the filter acts as a video capture source.

Target behavior:

```text
ZigVirtualCam.ax
  -> registered using regsvr32
  -> appears as "Zig Virtual Camera"
  -> outputs a generated RGB test pattern
  -> detected by DirectShow-capable apps such as OBS
```

---

## 2. Initial Project

We started with a C++ DirectShow virtual camera source filter project.

Important project files:

```text
build_manual_x64_msvc.bat
src\dllmain.cpp
src\VirtualCam.cpp
src\VirtualCam.h
src\VirtualCam.def
third_party\baseclasses\
```

The virtual camera code uses:

```text
CSource
CSourceStream
IAMStreamConfig
IKsPropertySet
```

The stream generates a moving RGB24 test pattern in `FillBuffer()`.

---

## 3. Getting Microsoft DirectShow BaseClasses

The project depends on Microsoft DirectShow BaseClasses.

We cloned Microsoft Windows Classic Samples:

```bat
cd C:\codes\zig
git clone https://github.com/microsoft/Windows-classic-samples.git
```

Then copied the BaseClasses folder:

```bat
xcopy /E /I /Y Samples\Win7Samples\multimedia\directshow\baseclasses ..\vcam\third_party\baseclasses
```

Expected result:

```text
C:\codes\zig\vcam\third_party\baseclasses\streams.h
C:\codes\zig\vcam\third_party\baseclasses\source.cpp
C:\codes\zig\vcam\third_party\baseclasses\wxdebug.cpp
C:\codes\zig\vcam\third_party\baseclasses\dllentry.cpp
...
```

---

## 4. Zig Build Script Problem

The original `build.zig` failed with Zig 0.16.0:

```text
error: no field or member function named 'addSharedLibrary' in 'Build'
```

This happened because Zig 0.16 changed the build system API.

Instead of fixing `build.zig`, we switched to a manual `zig c++` build script:

```text
build_manual_x64_msvc.bat
```

---

## 5. `.def` Export Problem

The original manual build attempted to use:

```bat
-Wl,/DEF:src\VirtualCam.def
```

or:

```bat
-Xlinker /DEF:src\VirtualCam.def
```

Zig 0.16 rejected the `.def` linker argument.

So we stopped using the `.def` export file and exported the COM functions directly using `__declspec(dllexport)`.

Patched functions:

```cpp
DllRegisterServer
DllUnregisterServer
DllGetClassObject
DllCanUnloadNow
```

Files involved:

```text
src\dllmain.cpp
third_party\baseclasses\dllentry.cpp
```

---

## 6. CopyMediaType Error

The compiler failed in `src\VirtualCam.cpp` because this was wrong:

```cpp
CopyMediaType(ppmt, &mt);
```

BaseClasses `CopyMediaType` expects `AM_MEDIA_TYPE*`, not `AM_MEDIA_TYPE**`.

We fixed it by using:

```cpp
*ppmt = CreateMediaType(&mt);
return *ppmt ? S_OK : E_OUTOFMEMORY;
```

and:

```cpp
*ppmt = CreateMediaType(&mt);
hr = *ppmt ? S_OK : E_OUTOFMEMORY;
```

This fixed `GetFormat()` and `GetStreamCaps()`.

---

## 7. Bad Newline Patch Mistake

One patch accidentally inserted literal text:

```text
`r`n
```

inside `src\VirtualCam.cpp`.

That produced errors like:

```text
error: expected expression
```

We fixed the accidental literal text and restored real newlines.

---

## 8. Zig libc++ / MSVC type_info Conflict

The build later failed with a conflict involving:

```text
vcruntime_typeinfo.h
libcxxabi/include/cxxabi.h
type_info
```

This happened because Zig tried to pull in Zig libc++/libc++abi while targeting MSVC.

We fixed it by adding these flags to the manual build:

```bat
-fno-exceptions
-fno-rtti
-nostdlib++
```

This stopped Zig from trying to build/link its own C++ runtime for this DirectShow project.

---

## 9. wxdebug.cpp Clang Compatibility Issue

The next failure was in Microsoft BaseClasses:

```text
third_party\baseclasses\wxdebug.cpp
CDisp::CDisp(pp);
```

Zig/Clang rejected this old MSVC-style constructor call.

We patched it to a Clang-compatible version:

```cpp
CDisp pinDisp(pp);
(void)StringCchCopy(m_String, NUMELMS(m_String), (LPCTSTR)pinDisp);
```

This allowed `wxdebug.cpp` to compile.

---

## 10. Missing Link Libraries and Source Files

The next failures were linker errors for Windows API functions such as:

```text
SetRect
SetWindowLongPtrW
RegOpenKeyExW
BitBlt
StretchDIBits
DirectDrawCreateFromDevice
llMulDiv
```

We fixed these by adding missing libraries:

```bat
-luser32
-lgdi32
-ladvapi32
-lddraw
-ldxguid
```

We also added missing BaseClasses source files:

```text
third_party\baseclasses\arithutil.cpp
third_party\baseclasses\ddmm.cpp
```

`arithutil.cpp` provides `llMulDiv`.

`ddmm.cpp` provides helpers for:

```text
DirectDrawCreateFromDevice
DirectDrawCreateFromDeviceEx
```

---

## 11. Final Successful Build

After the final build script update, this command succeeded:

```bat
cd C:\codes\zig\vcam
build_manual_x64_msvc.bat
```

Successful output:

```text
BUILD OK:
C:\codes\zig\vcam\zig-out\bin\ZigVirtualCam.ax
```

The output file is:

```text
zig-out\bin\ZigVirtualCam.ax
```

---

## 12. Registering the Camera

Registration initially failed with:

```text
0x80070005
```

That means:

```text
Access denied
```

The fix was to run `regsvr32` from an Administrator Command Prompt.

For the 64-bit build:

```bat
C:\Windows\System32\regsvr32.exe C:\codes\zig\vcam\zig-out\bin\ZigVirtualCam.ax
```

Important:

```text
System32\regsvr32.exe = 64-bit regsvr32
SysWOW64\regsvr32.exe = 32-bit regsvr32
```

Because our build is 64-bit, we used:

```text
C:\Windows\System32\regsvr32.exe
```

---

## 13. OBS Test

After registration, OBS detected:

```text
Zig Virtual Camera
```

This confirmed that the DirectShow `.ax` virtual camera was working.

OBS result:

```text
OBS detects Zig Virtual Camera: yes
```

---

## 14. Google Meet / Chrome Status

Chrome camera settings showed the device:

```text
chrome://settings/content/camera
```

The camera appeared there, which means Chrome can enumerate it.

However, Google Meet did not immediately show/use it.

Current likely causes:

```text
1. Google Meet permission/cache issue
2. Meet device list not refreshed
3. Meet rejects the current media type
4. The virtual camera currently only exposes RGB24 640x480 30 FPS
```

Possible next improvements:

```text
Add RGB32 support
Add YUY2 support
Add NV12 support
Improve IAMStreamConfig formats
Consider Media Foundation virtual camera support
```

---

## 15. Current Known Good Build Command

The current known-good build entry point is:

```bat
build_manual_x64_msvc.bat
```

Before rebuilding from scratch, run:

```bat
tools\complete_clean.bat
build_manual_x64_msvc.bat
```

---

## 16. Current Known Good Clean Command

A clean rebuild can be done with:

```bat
cd C:\codes\zig\vcam
tools\complete_clean.bat
build_manual_x64_msvc.bat
```

`complete_clean.bat` removes generated build output and Zig cache folders, but does not delete source files or `third_party\baseclasses`.
