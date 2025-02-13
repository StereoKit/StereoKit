# Navigate to the parent directory
Push-Location "$PSScriptRoot\.."

$SourceFolder = ".\StereoKitC"
$OutputFile   = ".\tools\stereokit.merged.txt"
$ExcludeDirs  = @(
	"StereoKitC_UWP",
	"lib\include"
)
$ExcludeFiles = @(
	"libraries\miniaudio.h",
	"libraries\miniaudio.cpp",
	"libraries\cgltf.h",
	"libraries\cgltf.cpp",
	"libraries\qoi.h",
	"libraries\qoi.cpp",
	"libraries\stb.cpp",
	"libraries\stb_truetype.h",
	"libraries\stb_image.h",
	"libraries\stb_image_write.h",
	"libraries\sokol_time.h",
	"libraries\sokol_time.cpp",
	"libraries\default_controller_l.h",
	"libraries\default_controller_r.h",
	"libraries\aileron_font_data.h"
)

& .\tools\ConcatenateFiles.ps1 `
	-SourceFolder $SourceFolder `
	-OutputFile $OutputFile `
	-ExcludeDirs $ExcludeDirs `
	-ExcludeFiles $ExcludeFiles

Pop-Location