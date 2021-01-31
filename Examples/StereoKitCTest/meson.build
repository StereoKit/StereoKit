stereokit_c_test_includes = include_directories('.', '../../StereoKitC')

src = [
	'main.cpp',
	'demo_basics.cpp',
	'demo_lines.cpp',
	'demo_sprites.cpp',
	'demo_ui.cpp',
	'scene.cpp'
]

executable(
	'stereokit_c_test_main',
	src,
	include_directories: stereokit_c_test_includes,
	dependencies: [stereokit_c]
)
