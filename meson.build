project('stereokit', 'cpp', default_options : ['warning_level=0', 'cpp_std=c++17'])
cppc = meson.get_compiler('cpp')
pkg = import('pkgconfig')
add_global_arguments('-D_DEBUG=true', language : 'cpp')

subdir('StereoKitC')
subdir('Examples')
