param(
    [switch]$upload = $false,
    [switch]$fast = $false,
    [switch]$skipTest = $false,
    [string]$key = ''
)

function Get-LineNumber { return $MyInvocation.ScriptLineNumber }
function Get-ScriptName { return $MyInvocation.ScriptName }

# In case we only want to build a subset of the package
$buildWindows    = $true
$buildWindowsUWP = $true
$buildLinux      = $true
$buildAndroid    = $true

###########################################
## Functions                             ##
###########################################

function Replace-In-File {
    param($file, $text, $with)

    ((Get-Content -path $file) -replace $text,$with) | Set-Content -path $file
}

###########################################

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

###########################################

function Build-Preset {
    param($preset, $presetName, [switch]$wsl = $false)

    Write-Host "--- Beginning build: $presetName ---" -ForegroundColor green
    if ($wsl -eq $true) { & wsl cmake --workflow --preset $preset }
    else                { & cmake --workflow --preset $preset }
    if ($LASTEXITCODE -ne 0) {
        Write-Host '--- $presetName build failed! Stopping build! ---' -ForegroundColor red
        Pop-Location
        exit
    }
    Write-Host "--- Finished building: $presetName ---" -ForegroundColor green
}

###########################################

function Build-Sizes {
    $size_x64        = (Get-Item "bin/distribute/bin/Win32/x64/Release/StereoKitC.dll").length
    $size_x64_linux  = (Get-Item "bin/distribute/bin/Linux/x64/release/libStereoKitC.so").length
    $size_x64_uwp    = (Get-Item "bin/distribute/bin/UWP/x64/Release/StereoKitC.dll").length
    $size_arm64      = (Get-Item "bin/distribute/bin/Android/arm64-v8a/release/libStereoKitC.so").length
    $size_arm64_linux= (Get-Item "bin/distribute/bin/Linux/arm64/release/libStereoKitC.so").length
    $size_arm64_uwp  = (Get-Item "bin/distribute/bin/UWP/ARM64/Release/StereoKitC.dll").length
    $size_arm_uwp    = (Get-Item "bin/distribute/bin/UWP/ARM/Release/StereoKitC.dll").length

    $text = (@"
## Build Sizes:

| Platform | Arch  | Size, kb | Size, bytes |
| -------- | ----- | -------- | ----------- |
| Win32    | x64   | {0,8:N0} | {1,11:N0} |
| UWP      | x64   | {2,8:N0} | {3,11:N0} |
| UWP      | ARM64 | {4,8:N0} | {5,11:N0} |
| UWP      | ARM   | {6,8:N0} | {7,11:N0} |
| Linux    | x64   | {8,8:N0} | {9,11:N0} |
| Linux    | ARM64 | {10,8:N0} | {11,11:N0} |
| Android  | ARM64 | {12,8:N0} | {13,11:N0} |
"@ -f ([math]::Round($size_x64        /1kb), $size_x64,
       [math]::Round($size_x64_uwp    /1kb), $size_x64_uwp,
       [math]::Round($size_arm64_uwp  /1kb), $size_arm64_uwp,
       [math]::Round($size_arm_uwp    /1kb), $size_arm_uwp,
       [math]::Round($size_x64_linux  /1kb), $size_x64_linux,
       [math]::Round($size_arm64_linux/1kb), $size_arm64_linux,
       [math]::Round($size_arm64      /1kb), $size_arm64
       ))

    return $text
}

###########################################
## Main                                  ##
###########################################

# Notify about our upload flag status 
if ($fast -eq $true -and $upload -eq $true) {
    Write-Host "Let's not upload a fast build, just in case! Try again without the fast flag :)" -ForegroundColor yellow
    exit
}
Write-Host 'Building... ' -NoNewline
if ($upload -eq $false) {
    Write-Host 'local only.' -ForegroundColor White
} else {
    Write-Host 'AND UPLOADING!' -ForegroundColor Green
}
if ($fast -eq $true) {
    Write-Host 'Making a "fast" build, incremental build issues may be present.'
}

# Switch to the right folder

Push-Location -Path "$PSScriptRoot\.."

#### Update Version #######################

# Print version, so we know we're building the right version right away
$fileData = Get-Content -path 'StereoKitC\stereokit.h' -Raw;
$fileData -match '#define SK_VERSION_MAJOR\s+(?<ver>\d+)' | Out-Null
$major = $Matches.ver
$fileData -match '#define SK_VERSION_MINOR\s+(?<ver>\d+)' | Out-Null
$minor = $Matches.ver
$fileData -match '#define SK_VERSION_PATCH\s+(?<ver>\d+)' | Out-Null
$patch = $Matches.ver
$fileData -match '#define SK_VERSION_PRERELEASE\s+(?<ver>\d+)' | Out-Null
$pre = $Matches.ver

$version = "$major.$minor.$patch"
if ($pre -ne 0) {
    $version = "$version-preview.$pre"
}

# Notify of build, and output the version
Write-Host @"
   _____ _                      _  __  _   
  / ____| |                    | |/ ( ) |  
 | (___ | |_ ___ _ __ ___  ___ |   / _| |_ 
  \___ \| __/ _ \  __/ _ \/   \|  < | | __|
  ____) | ||  __/ | |  __/ ( ) |   \| | |_ 
 |_____/ \__\___|_|  \___|\___/|_|\_\_|\__| 
"@ -NoNewline -ForegroundColor White
Write-Host "v$version`n" -ForegroundColor Cyan

# Ensure the version string for the package matches the StereoKit version
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text '<Version>(.*)</Version>' -with "<Version>$version</Version>"
Replace-In-File -file 'CMakeLists.txt' -text 'StereoKit VERSION "(.*)"' -with "StereoKit VERSION `"$major.$minor.$patch`""

#### Clean Project ########################

# Clean out the old files, do a full build
Write-Host 'Cleaning old files...'
if (Test-Path 'bin\distribute') {
    Remove-Item 'bin\distribute' -Recurse
}
if ($fast -eq $false) {
    if (Test-Path 'bin\intermediate\cmake') {
        Remove-Item 'bin\intermediate\cmake' -Recurse -Force
    }
}
Write-Host 'Cleaned'

#### Build Windows ########################

if ($buildWindows) {
    Write-Host @"

__      __          _               
\ \    / ( )_ _  __| |_____ __ _____
 \ \/\/ /| | ' \/ _  / _ \ V  V (_-<
  \_/\_/ |_|_||_\__ _\___/\_/\_//__/

"@ -ForegroundColor White

    Build-Preset -preset Win32x64Release -presetName 'Win32 x64'
    if ($buildWindowsUWP) {
        Build-Preset -preset Uwpx64Release   -presetName 'UWP x64'
        Build-Preset -preset UwpArm32Release -presetName 'UWP ARM32'
        Build-Preset -preset UwpArm64Release -presetName 'UWP ARM64'
    }
}

#### Build Linux ##########################
if ($buildLinux) {
    Write-Host @"

  _                   
 | |  (_)_ _ _  ___ __
 | |__| | ' \ || \ \ /
 |____|_|_||_\_,_/_\_\
                      
"@ -ForegroundColor White

    # Build-Preset -preset LinuxArm64Release -presetName 'Linux ARM64' -wsl
    Build-Preset -preset Linuxx64Release -presetName 'Linux x64' -wsl
}

#### Build Android ########################

if ($buildAndroid) {
    Write-Host @"

    _           _              _ 
   /_\  _ _  __| |_ _ ___( )__| |
  / _ \| ' \/ _' | '_/ _ \ / _' |
 /_/ \_\_||_\__,_|_| \___/_\__,_|
                      
"@ -ForegroundColor White

    #Build-Preset -preset Androidx64Release -presetName 'Android x64'
    Build-Preset -preset AndroidArm64Release -presetName 'Android arm64-v8a'
}

#### Run tests ########################

# Run tests!
if ($fast -eq $false -and $skipTest -eq $false) {
    Run-Tests
} else {
    Write-Host "`nSkipping tests for fast build!" -ForegroundColor yellow
}

#### Assemble NuGet Package ###############

Write-Host @"

  _  _       ___     _   
 | \| |_  _ / __|___| |_ 
 |    | || | (_ / -_)  _|
 |_|\_|\___|\___\___|\__|
                      
"@ -ForegroundColor White

Write-Host "--- Beginning build: NuGet package ---" -ForegroundColor green
# Turn on NuGet package generation, build, then turn it off again
$packageOff = '<GeneratePackageOnBuild>false</GeneratePackageOnBuild>'
$packageOn  = '<GeneratePackageOnBuild>true</GeneratePackageOnBuild>'
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOff -with $packageOn
& dotnet build StereoKit\StereoKit.csproj
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOn -with $packageOff
if ($LASTEXITCODE -ne 0) {
    Write-Host '--- NuGet build failed! Stopping build! ---' -ForegroundColor red
    Pop-Location
    exit
}
Write-Host "--- Finished building: NuGet package ---"-ForegroundColor green

#### Create Build Info File ###############

$build_size = Build-Sizes
$build_info = "# StereoKit v$version Build Information

$build_size"
Set-Content -path 'tools\BuildInfo.md' -value $build_info
Write-Host $build_info

#### Upload NuGet Package #################

if ($upload) {
    $key = Get-Key
    if ($key -ne '') {
        & dotnet nuget push "bin\StereoKit.$version.nupkg" -k $key -s https://api.nuget.org/v3/index.json
    } else {
        Write-Host 'No key, cancelling upload'
    }
}

Write-Host "Done!" -ForegroundColor green

Pop-Location