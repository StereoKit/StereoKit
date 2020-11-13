stereokit_c_includes = include_directories('.', './lib/include_no_win')

cppc = meson.get_compiler('cpp')

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

library(
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
	dependencies: [asset_types, libraries, systems, openxr, dl, reactphysics3d, gl, glew, glx, x11]
)
