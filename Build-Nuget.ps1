param(
    [switch]$noupload = $false,
    [string]$key = ''
)

$vsExe = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property productPath

function Get-Version {
    $fileData = Get-Content -path 'StereoKitC\stereokit.h' -Raw;

    $fileData -match '#define SK_VERSION_MAJOR\s+(?<ver>\d+)' | Out-Null
    $major = $Matches.ver
    $fileData -match '#define SK_VERSION_MINOR\s+(?<ver>\d+)' | Out-Null
    $minor = $Matches.ver
    $fileData -match '#define SK_VERSION_PATCH\s+(?<ver>\d+)' | Out-Null
    $patch = $Matches.ver
    $fileData -match '#define SK_VERSION_PRERELEASE\s+(?<ver>\d+)' | Out-Null
    $pre = $Matches.ver

    $result = "$major.$minor.$patch"
    if ($pre -ne 0) {
        $result = "$result-preview$pre"
    }

    return $result
}
function Replace-In-File {
    param($file, $text, $with)

    ((Get-Content -path $file) -replace $text,$with) | Set-Content -path $file
}
function Clean {
    & $vsExe 'StereoKit.sln' '/Clean' | Out-Null
}
function Build {
    param([parameter(Mandatory)][string] $mode)
    & $vsExe 'StereoKit.sln' '/Build' $mode '/Project' 'StereoKit' | Out-Null
}
function Get-Key {
    if ($key -ne '') {
        Set-Content -path '.nugetkey' -value $key.Trim()
    } elseif (Test-Path '.nugetkey') {
        $key = Get-Content -path '.nugetkey' -Raw
    } else {
        $key = Read-Host "Please enter the NuGet API key, or n to cancel"
        if ($key -eq 'n') {
            $key = ''
        } else {
            Set-Content -path '.nugetkey' -value $key.Trim()
        }
    }

    return $key.Trim()
}

Write-Output 'Beginning a full build!'

$version = Get-Version
Write-Output "v$version"
# Ensure the version string for the package matches the StereoKit version
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text '<Version>(.*)</Version>' -with "<Version>$version</Version>"

# Clean out the old files, do a full build
Clean
Write-Output 'Cleaned'

# Build ARM first
Build -mode "Release|ARM64"
Write-Output "Finished building: ARM64"

# Turn on NuGet package generation, build x64, then turn it off again
$packageOff = '<GeneratePackageOnBuild>false</GeneratePackageOnBuild>'
$packageOn  = '<GeneratePackageOnBuild>true</GeneratePackageOnBuild>'
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOff -with $packageOn
Build -mode "Release|X64"
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOn -with $packageOff
Write-Output "Finished building: X64"

if (-not $upload) {
    $key = Get-Key
    if ($key -ne '') {
        & dotnet nuget push "bin\StereoKit.$version.nupkg" -k $key -s https://api.nuget.org/v3/index.json
    } else {
        Write-Output 'No key, cancelling upload'
    }
}