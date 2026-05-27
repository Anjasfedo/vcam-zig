This ZIP contains only the changed files for the Zig 0.16 build workaround.

Copy/replace these into C:\codes\zig\vcam:

  build_manual_x64_msvc.bat         -> C:\codes\zig\vcam\build_manual_x64_msvc.bat
  src\dllmain.cpp                   -> C:\codes\zig\vcam\src\dllmain.cpp
  tools\patch_baseclasses_dllentry_exports.bat -> C:\codes\zig\vcam\tools\patch_baseclasses_dllentry_exports.bat

Then run:

  cd C:\codes\zig\vcam
  tools\patch_baseclasses_dllentry_exports.bat
  build_manual_x64_msvc.bat

Why there is a patch script instead of replacing dllentry.cpp directly:
Microsoft's baseclasses\dllentry.cpp comes from the Windows-classic-samples checkout you copied.
The script changes only the two COM export declarations needed by Zig 0.16, without replacing the whole Microsoft sample file.
