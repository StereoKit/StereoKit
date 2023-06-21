param(
    [int]$major,
    [int]$minor,
    [int]$patch,
    [int]$pre = 0
)

$version     = "$major.$minor.$patch"
$versionFull = $version
if ($pre -ne 0) {
    $versionFull = "$version-preview.$pre"
}

# This file is the source of truth for the version. It may even be the source
# of the version provided as an argument. Overwriting it may be redundant, but
# that's okay.
$filename = "$psScriptRoot\..\StereoKitC\stereokit.h"
$fileData = Get-Content -path $filename -Raw;
$fileData = $fileData -replace '#define SK_VERSION_MAJOR (.*)',"#define SK_VERSION_MAJOR $major"
$fileData = $fileData -replace '#define SK_VERSION_MINOR (.*)',"#define SK_VERSION_MINOR $minor"
$fileData = $fileData -replace '#define SK_VERSION_PATCH (.*)',"#define SK_VERSION_PATCH $patch"
$fileData = $fileData -replace '#define SK_VERSION_PRERELEASE (.*)',"#define SK_VERSION_PRERELEASE $pre"
[System.IO.File]::WriteAllText($filename, $fileData)

# NuGet package uses this version tag
$filename = "$psScriptRoot\..\StereoKit\StereoKit.csproj"
[System.IO.File]::WriteAllText($filename, ((Get-Content -Path $filename -raw) -replace '<Version>(.*)</Version>', "<Version>$versionFull</Version>"))

# CMake uses this version tag
$filename = "$psScriptRoot\..\CMakeLists.txt"
[System.IO.File]::WriteAllText($filename, ((Get-Content -Path $filename -raw) -replace 'StereoKit VERSION "(.*)"', "StereoKit VERSION `"$version`""))