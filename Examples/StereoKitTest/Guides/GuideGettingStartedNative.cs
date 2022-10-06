/// :CodeDoc: Guides 1 Getting Started Native
/// # Getting Started with StereoKit - Native Code
/// 
/// StereoKit's golden path is through C#, but the core of StereoKit is really
/// just a C compatible library! C# is just a 1st class binding to this C
/// library, so all of SK's core functionality is still accessible from native
/// code. This can be fantastic if you need the reliability of native code, or
/// the ability to easily interact with other native libraries.
/// 
/// However! StereoKit's core documentation is entirely focused on C#. Many C#
/// docs map 1:1 with the C code, so this is still the best reference, but the
/// C API is really **only recommended to more advanced developers**.
/// 
/// ## Native Template
/// 
/// The recommended way to get started with developing native StereoKit
/// applications is via the [CMake Template](https://github.com/StereoKit/SKTemplate-CMake).
/// Please see the template repository for up-to-date details and instructions!
/// This template is excellent for Linux and Windows development, and can be
/// fairly straightforward to use from VS Code.
/// 
/// ## Sample Code
/// 
/// StereoKit's C API can be found in 2 different .h files, [stereokit.h](https://github.com/StereoKit/StereoKit/blob/master/StereoKitC/stereokit.h)
/// for all the main functions, and [stereokit_ui.h](https://github.com/StereoKit/StereoKit/blob/master/StereoKitC/stereokit_ui.h)
/// for the user interface.
/// 
/// There is also some sample code available in the [StereoKitCTest project](https://github.com/StereoKit/StereoKit/tree/master/Examples/StereoKitCTest)
/// that can be used for reference! It's not as complete as the C# samples, but
/// should help point you in the right direction for usage patterns and
/// translation from C# docs.
/// 
/// ## Other Languages
/// 
/// Since StereoKit is a C API, it's pretty easy to bind to other languages!
/// While there are no official bindings other than C#, there are some partial
/// examples for [Zig](https://github.com/StereoKit/StereoKit/tree/master/Examples/StereoKitZig)
/// and [V](https://github.com/StereoKit/StereoKit/tree/master/Examples/StereoKitV),
/// as well as a community driven effort to bind with [rust](https://crates.io/crates/stereokit)!
/// 
/// :End: