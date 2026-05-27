const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addSharedLibrary(.{
        .name = "ZigVirtualCam",
        .target = target,
        .optimize = optimize,
    });

    lib.linkLibCpp();

    lib.addIncludePath(b.path("src"));
    lib.addIncludePath(b.path("third_party/baseclasses"));

    lib.addCSourceFiles(.{
        .files = &.{
            "src/dllmain.cpp",
            "src/VirtualCam.cpp",

            // Copy these DirectShow BaseClasses files into third_party/baseclasses.
            // The exact list can vary slightly depending on the SDK/sample version.
            "third_party/baseclasses/source.cpp",
            "third_party/baseclasses/wxdebug.cpp",
            "third_party/baseclasses/wxlist.cpp",
            "third_party/baseclasses/wxutil.cpp",
            "third_party/baseclasses/combase.cpp",
            "third_party/baseclasses/dllentry.cpp",
            "third_party/baseclasses/mtype.cpp",
            "third_party/baseclasses/renbase.cpp",
            "third_party/baseclasses/amextra.cpp",
            "third_party/baseclasses/ctlutil.cpp",
            "third_party/baseclasses/transfrm.cpp",
            "third_party/baseclasses/pstream.cpp",
            "third_party/baseclasses/outputq.cpp",
            "third_party/baseclasses/pullpin.cpp",
        },
        .flags = &.{
            "-std=c++17",
            "-DUNICODE",
            "-D_UNICODE",
            "-DWIN32_LEAN_AND_MEAN",
            "-D_CRT_SECURE_NO_WARNINGS",
        },
    });

    // If your Zig version rejects this, see README.md for the manual zig c++ command.
    lib.addObjectFile(b.path("src/VirtualCam.def"));

    lib.linkSystemLibrary("strmiids");
    lib.linkSystemLibrary("ole32");
    lib.linkSystemLibrary("oleaut32");
    lib.linkSystemLibrary("uuid");
    lib.linkSystemLibrary("winmm");

    b.installArtifact(lib);
}
