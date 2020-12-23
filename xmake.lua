add_rules("mode.release", "mode.debug")

--------------------------------------------------

package("reactphysics3d")
    set_homepage("http://www.reactphysics3d.com/")
    set_description("Open source C++ physics engine library in 3D")

    add_urls("https://github.com/DanielChappuis/reactphysics3d/releases/download/v$(version)/reactphysics3d-$(version).tar.gz")
    
    on_install("linux", "windows", "android", function (package)
        import("package.tools.cmake").install(package, {"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"})
    end)
package_end()

--------------------------------------------------

package("openxr")
    set_homepage("https://github.com/KhronosGroup/OpenXR-SDK")
    set_description("Generated headers and sources for OpenXR loader.")

    add_urls("https://github.com/KhronosGroup/OpenXR-SDK/archive/release-$(version).tar.gz")
    
    on_install("linux", "windows", "android", function (package)
        import("package.tools.cmake").install(package, {"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"})
    end)
package_end()

--------------------------------------------------

add_requires("reactphysics3d 0.8.0", {verify = false, configs = {vs_runtime="MD"}})
add_requires("openxr 1.0.13",        {verify = false, configs = {vs_runtime="MD"}})

target("StereoKitC")
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

    add_packages("reactphysics3d", "openxr")
    if is_plat("linux", "android") then 
        add_links("OpenGL", "GLEW", "GLX", "X11")
    end

    on_load(function (target)
        if is_plat("linux", "android") then
            target:add(find_packages("GL", "GLEW", "glx", "X11"))
        end
    end)

    -- This doesn't actually work yet either
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
        set_kind("binary")
        add_files("Examples/StereoKitCTest/*.cpp")
        add_includedirs("StereoKitC/")
        add_deps("StereoKitC")
end