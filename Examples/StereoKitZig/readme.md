# Experimental Zig Support

Since Zig plays nicely with c headers and libraries, it's pretty trivial to work with StereoKit from Zig! I'm no Zig expert, but I got it all up and running on Windows with just these two files.

These samples were built and tested using Zig 0.6.0 on Windows x64.

## Setup

Build the x64 release build of StereoKitC through Visual Studio, this will create files in the `bin\distribute` folder, which includes header files, lib files, and dlls.

Then, you'll need [Zig](https://ziglang.org/download/)! Store it someplace nice (I put it in `C:\Tools\Zig)`, and either remember that folder for console commands, or add that folder to your PATH.

Then you can use this in the command line to compile and execute the application. See [build.zig](build.zig) for the build process, and [main.zig](main.zig) for the application code!

`zig build run`

Where `zig` is the zig exe, `build` is the command, and `run` is an option coded into the `build.zig` file.