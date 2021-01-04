add_rules("mode.release", "mode.debug")

--------------------------------------------------
--[[
package("reactphysics3d")
    set_homepage("http://www.reactphysics3d.com/")
    set_description("Open source C++ physics engine library in 3D")

    add_urls("https://github.com/DanielChappuis/reactphysics3d/releases/download/v$(version)/reactphysics3d-$(version).tar.gz")
    
    on_install("linux", "windows", "android", function (package)
        import("package.tools.cmake").install(package, {"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"})
    end)
package_end()
]]--
--------------------------------------------------

package("openxr")
    set_homepage("https://github.com/KhronosGroup/OpenXR-SDK")
    set_description("Generated headers and sources for OpenXR loader.")

    add_urls("https://github.com/KhronosGroup/OpenXR-SDK/archive/release-$(version).tar.gz")
    
    on_install("linux", "windows", "android", function (package)
        import("package.tools.cmake").install(package) -- , {"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"})
    end)
package_end()

--------------------------------------------------

--add_requires("reactphysics3d 0.8.0", {verify = false, configs = {vs_runtime="MD"}})

-- On Android, we have a precompiled binary provided by Oculus
if not is_plat("android") then
    add_requires("openxr 1.0.13", {verify = false, configs = {vs_runtime="MD"}})
end

option("uwp")
    set_default(false)
    set_showmenu("true")
    set_description("Build for UWP")
    set_values(true, false)
    add_defines("WINDOWS_UWP", "WINAPI_FAMILY=WINAPI_FAMILY_APP")
    
target("StereoKitC")
    add_options("uwp")
    set_version("0.3.0")
    set_kind("shared")
    if (is_plat("windows")) then
        set_languages("cxx17")
    else
        set_languages("cxx11")
    end

    add_files("StereoKitC/*.cpp") 
    add_files("StereoKitC/libraries/*.cpp") 
    add_files("StereoKitC/systems/*.cpp") 
    add_files("StereoKitC/systems/hand/*.cpp") 
    add_files("StereoKitC/systems/platform/*.cpp") 
    add_files("StereoKitC/asset_types/*.cpp")

    add_includedirs("StereoKitC/lib/include")
    add_includedirs("StereoKitC/lib/include_no_win")

    if has_config("uwp") then
        add_defines("_WINRT_DLL", "_WINDLL", "__WRL_NO_DEFAULT_LIB__")
    end

    --add_packages("reactphysics3d")
    if is_plat("android") then
        add_linkdirs("StereoKitC/lib/bin/$(arch)/$(mode)")
        add_links("openxr_loader")
    else
        add_packages("openxr")
    end

    on_load(function (target)
        if is_plat("linux") then
            target:add(find_packages("GL", "GLEW", "glx", "X11"))
        elseif is_plat("android") then
            target:add(find_packages("EGL", "OpenSLES"))
        end
    end)

    -- This doesn't actually work yet
    if is_plat("wasm") then
        add_cxflags("-msimd128", "-msse4", "-s", "SIMD=1")
    end

    if is_plat("windows") then
        add_cxflags(is_mode("debug") and "/MDd" or "/MD")
        add_links("windowsapp", "user32")
    end

--------------------------------------------------

if is_plat("linux", "windows") then
    target("StereoKitCTest")
        add_options("uwp")
        set_kind("binary")
        add_files("Examples/StereoKitCTest/*.cpp")
        add_includedirs("StereoKitC/")
        add_deps("StereoKitC")
end

--------------------------------------------------
--[[
if is_plat("android") then
    target("StereoKitCTest_Android")
        set_kind("binary")
        add_files("Examples/StereoKitCTest_Android/StereoKitCTest_Android.NativeActivity/*.cpp")
        add_files("Examples/StereoKitCTest_Android/StereoKitCTest_Android.NativeActivity/*.c")
        add_includedirs("StereoKitC/")
        add_deps("StereoKitC")
end
]]--