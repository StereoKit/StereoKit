libraries_includes = include_directories('.', '../lib/include_no_win')#, '../lib/include')

lib_libraries = static_library(
	'libraries',
	files(
		'isac_spatial_sound.cpp',
		'miniz.cpp',
		'ofbx.cpp',
		'sk_gpu.cpp',
		'stref.cpp',
		'ferr_hash.cpp'
	),
	include_directories: [libraries_includes]
)

libraries = declare_dependency(
	include_directories: libraries_includes,
	link_with: lib_libraries
)
