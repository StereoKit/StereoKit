# Print version, so we know we're building the right version right away
$fileData = Get-Content -path "$PSScriptRoot\..\StereoKitC\stereokit.h" -Raw;
$fileData -match '#define SK_VERSION_MAJOR\s+(?<ver>\d+)' | Out-Null
$major = $Matches.ver
$fileData -match '#define SK_VERSION_MINOR\s+(?<ver>\d+)' | Out-Null
$minor = $Matches.ver
$fileData -match '#define SK_VERSION_PATCH\s+(?<ver>\d+)' | Out-Null
$patch = $Matches.ver
$fileData -match '#define SK_VERSION_PRERELEASE\s+(?<ver>\d+)' | Out-Null
$pre = $Matches.ver

$str = "$major.$minor.$patch"
if ($pre -ne 0) {
    $str = "$str-preview.$pre"
}

return @{
    'str' = $str; 
    'major' = $major;
    'minor' = $minor;
    'patch' = $patch; 
    'pre' = $pre }