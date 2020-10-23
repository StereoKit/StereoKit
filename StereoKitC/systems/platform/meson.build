platform_includes = include_directories('.', '../../lib/include_no_win')#, '../../lib/include')

lib_platform = static_library(
	'platform',
	files(
		'flatscreen_input.cpp',
		'openxr_input.cpp',
		'openxr_view.cpp',
		'openxr.cpp',
		'platform_utils.cpp'
	),
	include_directories: [platform_includes]
)

platform = declare_dependency(
	include_directories: platform_includes,
	link_with: lib_platform
)
