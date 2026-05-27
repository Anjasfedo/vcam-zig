# Zig Virtual Camera DirectShow Starter

This is a minimal Windows DirectShow virtual camera starter project.
It builds a COM DLL / DirectShow source filter that can be output as `ZigVirtualCam.ax`.
The camera generates a moving RGB24 test pattern at 640x480, 30 FPS.

## Important

This ZIP does **not** include Microsoft's DirectShow BaseClasses source files.
You must copy them into:

```text
third_party/baseclasses/
```

You need `streams.h` and implementation files such as `source.cpp`, `combase.cpp`, `dllentry.cpp`, `mtype.cpp`, etc.

## Requirements

- Windows
- Zig installed and in PATH
- Windows SDK installed
- DirectShow BaseClasses source copied into `third_party/baseclasses`
- Administrator terminal for registration

## Build with Zig build

```bat
zig build -Dtarget=x86_64-windows-msvc
```

Expected output location depends on Zig version, usually under:

```text
zig-out/bin/ZigVirtualCam.dll
```

Rename/copy it to:

```text
ZigVirtualCam.ax
```

A `.ax` is just a DirectShow COM DLL with a conventional extension.

## Manual build

You can also try:

```bat
build_manual_x64_msvc.bat
```

This attempts to produce:

```text
ZigVirtualCam.ax
```

## Register

Run an Administrator terminal in the folder containing `ZigVirtualCam.ax`:

```bat
C:\Windows\System32\regsvr32.exe ZigVirtualCam.ax
```

Or run:

```bat
register_x64_admin.bat
```

## Unregister

```bat
C:\Windows\System32\regsvr32.exe /u ZigVirtualCam.ax
```

Or run:

```bat
unregister_x64_admin.bat
```

## Test

Try one of these:

- GraphStudioNext
- OBS Studio: Video Capture Device -> Zig Virtual Camera
- VLC capture device
- OpenCV VideoCapture

## 32-bit apps

A 32-bit app requires a 32-bit `.ax` filter.
Build with:

```bat
zig build -Dtarget=x86-windows-msvc
```

Register the 32-bit build with:

```bat
C:\Windows\SysWOW64\regsvr32.exe ZigVirtualCam.ax
```

## What to modify next

The current frame generator is in:

```cpp
CZigVirtualCamStream::FillBuffer(IMediaSample* sample)
```

Replace the gradient generation with a copy from shared memory, a named pipe, or another IPC source if you want an external app to feed frames into the camera.

## Notes

Some modern apps may not list old DirectShow virtual cameras. Those apps may use Media Foundation only. For broad compatibility, a production project may need both a DirectShow filter and a Media Foundation virtual camera implementation.
