stereokit_c_includes = include_directories('.', './lib/include_no_win')

openxr = dependency('openxr', required: true)
dl = cppc.find_library('dl')
reactphysics3d = dependency('ReactPhysics3D', required: true)
gl = dependency('GL', required: true)
glew = dependency('GLEW', required: true)
glx = dependency('glx', required: true)
x11 = dependency('X11', required: true)

subdir('asset_types')
# subdir('lib')
subdir('libraries')
# subdir('shaders_builtin')
subdir('systems')

lib_stereokit_c = library(
	'stereokit_c',
	files(
		'color.cpp',
		'hierarchy.cpp',
		'intersect.cpp',
		'log.cpp',
		'sk_math.cpp',
		'spherical_harmonics.cpp',
		'stereokit_ui.cpp',
		'stereokit.cpp'
	),
	include_directories: [stereokit_c_includes],
	dependencies: [asset_types, libraries, systems, openxr, dl, reactphysics3d, gl, glew, glx, x11],
	version: '0.2.1',
	soversion: '0',
	install: true
)

stereokit_c = declare_dependency(
	include_directories: stereokit_c_includes,
	link_with: lib_stereokit_c
)

install_headers(['stereokit.h', 'stereokit_ui.h', 'sk_math.h'])
pkg.generate(lib_stereokit_c, filebase: 'stereokit_c', name: 'StereoKitC', url: 'https://github.com/maluoi/StereoKit')
