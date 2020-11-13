asset_includes = include_directories('.', '../lib/include_no_win')

lib_asset_types = static_library(
	'asset_types',
	files(
		'assets.cpp',
		'font.cpp',
		'material.cpp',
		'mesh.cpp',
		'model_fbx.cpp',
		'model_gltf.cpp',
		'model_obj.cpp',
		'model_stl.cpp',
		'model.cpp',
		'shader.cpp',
		'sound.cpp',
		'sprite.cpp',
		'texture.cpp'
	),
	include_directories: [asset_includes]
)

asset_types = declare_dependency(
	include_directories: asset_includes,
	link_with: lib_asset_types
)
