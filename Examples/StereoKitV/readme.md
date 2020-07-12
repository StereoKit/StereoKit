# Experimental V Support

It is possible to use StereoKit from V, but requires a bit more work for full support. A small module would need maintained separately that would translate StereoKit functions into V! I may do this at some point, but it's not a priority. This is a quick example of how it could work!

## Setup

Build the x64 release build of StereoKitC through Visual Studio, this will create files in the `bin\distribute` folder, which includes header files, lib files, and dlls.

Then, you'll need [V](https://vlang.io/)! Store it someplace nice (I put it in `C:\Tools\V)`, and either remember that folder for console commands, or add that folder to your PATH.

Then you can use this in the command line to compile and execute the application. See [build.v](build.v) for the build process, and [main.v](main.v) for the application code!

`v run build.v`

While you could directly call `v run main.v` instead, the `build.v` file takes care of copying the DLLs over to the appropriate folder. `build.v` also tosses in whatever extra .v files might be in the folder as well!