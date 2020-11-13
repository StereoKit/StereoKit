subdir('hand')
subdir('platform')

systems_includes = include_directories('.', '../lib/include_no_win', '../lib/include')

lib_systems = static_library(
	'systems',
	files(
		'defaults.cpp',
		'input.cpp',
		'line_drawer.cpp',
		'physics.cpp',
		'render_sort.cpp',
		'render.cpp',
		'sprite_drawer.cpp',
		'system.cpp',
		'text.cpp'
	),
	include_directories: [systems_includes],
	dependencies: [hand, platform]
)

systems = declare_dependency(
	include_directories: systems_includes,
	link_with: lib_systems
)
