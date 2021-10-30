add_rules("mode.release", "mode.debug")

--------------------------------------------------

package("reactphysics3d")
    set_homepage("http://www.reactphysics3d.com/")
    set_description("Open source C++ physics engine library in 3D")

    add_urls("https://github.com/DanielChappuis/reactphysics3d/releases/download/v$(version)/reactphysics3d-$(version).tar.gz")
    
    add_deps("cmake")
    on_install("linux", "windows", "android", function (package)
        import("package.tools.cmake").install(package, {"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"})
    end)
package_end()

--------------------------------------------------

package("openxr_loader")
    set_homepage("https://github.com/KhronosGroup/OpenXR-SDK")
    set_description("Generated headers and sources for OpenXR loader.")
    
    add_urls("https://github.com/KhronosGroup/OpenXR-SDK/archive/release-$(version).tar.gz")
    
    add_deps("cmake")

    if is_plat("linux") then
        add_syslinks("stdc++fs", "jsoncpp")
    end
    
    on_install("linux", "windows", "android", function (package)
        import("package.tools.cmake").install(package, {"-DDYNAMIC_LOADER=OFF"})
    end)
package_end()

--------------------------------------------------

-- On Android, we have a precompiled binary provided by Oculus
if not is_plat("android") then
    add_requires("openxr_loader 1.0.17", {verify = false, configs = {vs_runtime="MD", shared=false}})
end
add_requires("reactphysics3d 0.8.0", {verify = false, configs = {vs_runtime="MD", shared=false}})

option("uwp")
    set_default(false)
    set_showmenu("true")
    set_description("Build for UWP")
    set_values(true, false)
    add_defines("WINDOWS_UWP", "WINAPI_FAMILY=WINAPI_FAMILY_APP")
    
option("linux-graphics-backend")
    set_default("GLX")
    set_showmenu("true")
    set_description("Graphics backend on Linux")
    set_values("GLX", "EGL")

target("StereoKitC")
    add_options("uwp")
    add_options("linux-graphics-backend")
    set_version("0.3.4")
    set_kind("shared")
    set_symbols("debug")
    if is_plat("windows") then
        set_languages("cxx17")
        add_cxflags(is_mode("debug") and "/MDd" or "/MD")
        add_links("windowsapp", "user32")
        if has_config("uwp") then
            add_defines("_WINRT_DLL", "_WINDLL", "__WRL_NO_DEFAULT_LIB__")
        end
    else
        set_languages("cxx11")
    end

    -- 2.5.3 is needed for utils.install.pkgconfig_importfiles
    set_xmakever("2.5.3") 
    add_rules("utils.install.pkgconfig_importfiles")
    add_headerfiles("StereoKitC/stereokit*.h")

    add_files("StereoKitC/*.cpp") 
    add_files("StereoKitC/libraries/*.cpp") 
    add_files("StereoKitC/tools/*.cpp") 
    add_files("StereoKitC/systems/*.cpp") 
    add_files("StereoKitC/systems/hand/*.cpp") 
    add_files("StereoKitC/systems/platform/*.cpp") 
    add_files("StereoKitC/asset_types/*.cpp")
    add_includedirs("StereoKitC/lib/include")
    add_includedirs("StereoKitC/lib/include_no_win")

    add_packages("reactphysics3d")
    -- On Android, we have a precompiled binary provided by Oculus
    if is_plat("android") then
        add_linkdirs("StereoKitC/lib/bin/$(arch)/$(mode)")
        add_links("openxr_loader")
    else
        add_packages("openxr_loader")
    end

    -- Pick our flavor of OpenGL
    if is_plat("linux") then
		if is_config("linux-graphics-backend", "EGL") then
        	add_links("EGL", "GLX", "fontconfig", "pthread")
			add_defines("SKG_LINUX_EGL")
		elseif is_config("linux-graphics-backend", "GLX") then
        	add_links("GL", "GLEW", "GLX", "fontconfig", "X11", "pthread")
			add_defines("SKG_LINUX_GLX")
		end
    elseif is_plat("android") then
        add_links("EGL", "OpenSLES", "android")
    end

    -- Emscripten stuff, this doesn't actually work yet
    if is_plat("wasm") then
        add_cxflags("-msimd128", "-msse4", "-s", "SIMD=1")
    end

    -- Copy finished files over to the bin directory
    after_build(function(target)
        if is_plat("windows") then
            dist_os = has_config("uwp") and "UWP" or "Win32"
        else
            dist_os = "$(os)"
        end
        build_folder = target:targetdir().."/"
        dist_folder  = "$(projectdir)/bin/distribute/bin/"..dist_os.."/$(arch)/$(mode)/"

        print("Copying binary files from "..build_folder.." to "..dist_folder)

        os.cp(build_folder.."*.dll", dist_folder)
        os.cp(build_folder.."*.so",  dist_folder)
        os.cp(build_folder.."*.lib", dist_folder)
        os.cp(build_folder.."*.a",   dist_folder)
        os.cp(build_folder.."*.pdb", dist_folder)
        os.cp(build_folder.."*.sym", dist_folder)
        -- Oculus' pre-built OpenXR loader
        if is_plat("android") then
            os.cp("StereoKitC/lib/bin/$(arch)/$(mode)/libopenxr_loader.so", dist_folder)
        end

        os.cp("$(projectdir)/StereoKitC/stereokit.h",    "$(projectdir)/bin/distribute/include/")
        os.cp("$(projectdir)/StereoKitC/stereokit_ui.h", "$(projectdir)/bin/distribute/include/")
    end)

--------------------------------------------------

option("tests")
    set_default(true)
    set_showmenu("true")
    set_description("Build native test project")
    set_values(true, false)

if has_config("tests") and is_plat("linux", "windows") then
    target("StereoKitCTest")
        add_options("uwp")
        set_kind("binary")
        add_files("Examples/StereoKitCTest/*.cpp")
        add_includedirs("StereoKitC/")
        add_deps("StereoKitC")

        after_build(function (target)
            local assets_folder = path.join(target:targetdir(), "Assets")
            os.cp("Examples/Assets/", assets_folder)
        end)
end

--------------------------------------------------
