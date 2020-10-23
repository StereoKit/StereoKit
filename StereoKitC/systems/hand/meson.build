hand_includes = include_directories('.', '../../lib/include_no_win')#, '../../lib/include')

lib_hand = static_library(
	'hand',
	files(
		'hand_mouse.cpp',
		'hand_override.cpp',
		'hand_oxr_articulated.cpp',
		'hand_oxr_controller.cpp',
		'input_hand.cpp'
	),
	include_directories: [hand_includes]
)

hand = declare_dependency(
	include_directories: hand_includes,
	link_with: lib_hand
)
