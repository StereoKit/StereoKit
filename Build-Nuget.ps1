param(
    [switch]$upload = $false,
    [switch]$fast = $false,
    [string]$key = ''
)

$vsExe = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property productPath
$vsExe = [io.path]::ChangeExtension($vsExe, '.com')

###########################################
## Functions                             ##
###########################################

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
        $result = "$result-preview.$pre"
    }

    return $result
}

###########################################

function Replace-In-File {
    param($file, $text, $with)

    ((Get-Content -path $file) -replace $text,$with) | Set-Content -path $file
}

###########################################

function Build {
    param([parameter(Mandatory)][string] $mode, [parameter(Mandatory)][string] $project)
    & $vsExe 'StereoKit.sln' '/Build' $mode '/Project' $project | Write-Host
    return $LASTEXITCODE
}

###########################################

function Test {
    & $vsExe 'StereoKit.sln' '/Build' 'Release|X64' '/Project' 'StereoKitDocumenter' | Out-Null
    if ($LASTEXITCODE -ne 0) {
        return $LASTEXITCODE
    }
    & 'cd' 'StereoKitDocumenter/bin/Release/';
    & '.\StereoKitDocumenter.exe' | Write-Host
    & 'cd' '../../../';
    return $LASTEXITCODE
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

function Build-Sizes {
    $size_x64       = (Get-Item "bin/distribute/bin/Win32/x64/Release/StereoKitC.dll").length
    $size_x64_linux = (Get-Item "bin/distribute/bin/linux/x64/release/libStereoKitC.so").length
    $size_x64_uwp   = (Get-Item "bin/distribute/bin/UWP/x64/Release/StereoKitC.dll").length
    $size_arm64     = (Get-Item "bin/distribute/bin/android/arm64-v8a/release/libStereoKitC.so").length
    $size_arm64_uwp = (Get-Item "bin/distribute/bin/UWP/ARM64/Release/StereoKitC.dll").length
    $size_arm_uwp   = (Get-Item "bin/distribute/bin/UWP/ARM/Release/StereoKitC.dll").length

    $text = (@"
## Build Sizes:

| Platform | Arch  | Size, kb | Size, bytes |
| -------- | ----- | -------- | ----------- |
| Win32    | x64   | {0,8:N0} | {1,11:N0} |
| Linux    | x64   | {2,8:N0} | {3,11:N0} |
| UWP      | x64   | {4,8:N0} | {5,11:N0} |
| UWP      | ARM64 | {6,8:N0} | {7,11:N0} |
| Android  | ARM64 | {8,8:N0} | {9,11:N0} |
| UWP      | ARM   | {10,8:N0} | {11,11:N0} |
"@ -f ([math]::Round($size_x64/1kb), $size_x64,
       [math]::Round($size_x64_linux/1kb), $size_x64_linux,
       [math]::Round($size_x64_uwp/1kb), $size_x64_uwp,
       [math]::Round($size_arm64_uwp/1kb), $size_arm64_uwp,
       [math]::Round($size_arm64/1kb), $size_arm64,
       [math]::Round($size_arm_uwp/1kb), $size_arm_uwp))

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

#### Update Version #######################

# Print version, so we know we're building the right version right away
$version = Get-Version

# Notify of build, and output the version
Write-Host @"
   _____ _                      _  ___ _   
  / ____| |                    | |/ (_) |  
 | (___ | |_ ___ _ __ ___  ___ | ' / _| |_ 
  \___ \| __/ _ \ '__/ _ \/ _ \|  < | | __|
  ____) | ||  __/ | |  __/ (_) | . \| | |_ 
 |_____/ \__\___|_|  \___|\___/|_|\_\_|\__| 
"@ -NoNewline -ForegroundColor White
Write-Host "v$version`n" -ForegroundColor Cyan

# Ensure the version string for the package matches the StereoKit version
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text '<Version>(.*)</Version>' -with "<Version>$version</Version>"
Replace-In-File -file 'xmake.lua' -text 'set_version(.*)' -with "set_version(`"$version`")"

#### Clean Project ########################

# Clean out the old files, do a full build
if (Test-Path 'bin\distribute') {
    Remove-Item 'bin\distribute' -Recurse
}
Write-Host 'Cleaning old files...'
if ($fast -eq $false) {
    & $vsExe 'StereoKit.sln' '/Clean' 'Release|X64' | Out-Null
    Write-Host '..cleaned Release x64'
    & $vsExe 'StereoKit.sln' '/Clean' 'Release|ARM64' | Out-Null
    Write-Host '..cleaned Release ARM64'
    & $vsExe 'StereoKit.sln' '/Clean' 'Release|ARM' | Out-Null
    Write-Host '..cleaned Release ARM'
}
Write-Host 'Cleaned'

#### Build Windows ########################

Write-Host @"

__      ___         _               
\ \    / (_)_ _  __| |_____ __ _____
 \ \/\/ /| | ' \/ _` / _ \ V  V (_-<
  \_/\_/ |_|_||_\__,_\___/\_/\_//__/

"@ -ForegroundColor White

# Build Win32 first
Write-Host "--- Beginning build: Win32 x64 ---" -ForegroundColor green
$result = Build -mode "Release|X64" -project "StereoKitC"
if ($result -ne 0) {
    Write-Host '--- Win32 x64 build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host "--- Finished building: Win32 x64 ---" -ForegroundColor green
#Write-Host "--- Beginning build: Win32 ARM64 ---" -ForegroundColor green
#$result = Build -mode "Release|ARM64" -project "StereoKitC"
#if ($result -ne 0) {
#    Write-Host '--- Win32 ARM64 build failed! Stopping build! ---' -ForegroundColor red
#    exit
#}
#Write-Host "--- Finished building: Win32 ARM64 ---" -ForegroundColor green

# Build UWP next
Write-Host "--- Beginning build: UWP x64 ---" -ForegroundColor green
$result = Build -mode "Release|X64" -project "StereoKitC_UWP"
if ($result -ne 0) {
    Write-Host '--- UWP x64 build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host "--- Finished building: UWP x64 ---" -ForegroundColor green
Write-Host "--- Beginning build: UWP ARM64 ---" -ForegroundColor green
$result = Build -mode "Release|ARM64" -project "StereoKitC_UWP"
if ($result -ne 0) {
    Write-Host '--- UWP ARM64 build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host "--- Finished building: UWP ARM64 ---" -ForegroundColor green
Write-Host "--- Beginning build: UWP ARM ---" -ForegroundColor green
$result = Build -mode "Release|ARM" -project "StereoKitC_UWP"
if ($result -ne 0) {
    Write-Host '--- UWP ARM build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host "--- Finished building: UWP ARM ---" -ForegroundColor green

#### Execute Windows Tests ########################

# Run tests!
if ($fast -eq $false) {
    Write-Host "`nRunning Windows Tests!"
    if ( Test -ne 0 ) {
        Write-Host '--- Tests failed! Stopping build! ---' -ForegroundColor red
        exit
    }
    Write-Host 'Tests passed!' -ForegroundColor green
} else {
    Write-Host "`nSkipping tests for fast build!" -ForegroundColor yellow
}

#### Build Linux ##########################

Write-Host @"

  _    _              
 | |  (_)_ _ _  ___ __
 | |__| | ' \ || \ \ /
 |____|_|_||_\_,_/_\_\
                      
"@ -ForegroundColor White

# Linux, via WSL
Write-Host '--- Beginning WSL build: Linux x64 ---' -ForegroundColor green
if ($fast -eq $false) {
    cmd /c "wsl cd /mnt/c/Data/Repositories/StereoKit ; xmake f -p linux -a x64 -m release ; xmake -r"
} else {
    cmd /c "wsl cd /mnt/c/Data/Repositories/StereoKit ; xmake f -p linux -a x64 -m release ; xmake"
}
if ($LASTEXITCODE -ne 0) {
    Write-Host '--- Linux build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host '--- Finished building: Linux x64 ---' -ForegroundColor green

#### Build Android ########################

Write-Host @"

    _           _         _    _ 
   /_\  _ _  __| |_ _ ___(_)__| |
  / _ \| ' \/ _` | '_/ _ \ / _` |
 /_/ \_\_||_\__,_|_| \___/_\__,_|
                      
"@ -ForegroundColor White

# Do cross platform build code first
Write-Host '--- Beginning build: Android arm64-v8a' -ForegroundColor green
xmake f -p android -a arm64-v8a -m release
if ($fast -eq $false) {
    xmake -r
} else {
    xmake
}
if ($LASTEXITCODE -ne 0) {
    Write-Host '--- Android build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host '--- Finished building: Android arm64-v8a ---' -ForegroundColor green

#### Assemble NuGet Package ###############

Write-Host @"

  _  _       ___     _   
 | \| |_  _ / __|___| |_ 
 | .` | || | (_ / -_)  _|
 |_|\_|\_,_|\___\___|\__|
                      
"@ -ForegroundColor White

Write-Host "--- Beginning build: NuGet package ---" -ForegroundColor green
# Turn on NuGet package generation, build, then turn it off again
$packageOff = '<GeneratePackageOnBuild>false</GeneratePackageOnBuild>'
$packageOn  = '<GeneratePackageOnBuild>true</GeneratePackageOnBuild>'
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOff -with $packageOn
$result = Build -mode "Release|Any CPU" -project "StereoKit"
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOn -with $packageOff
if ($result -ne 0) {
    Write-Host '--- NuGet build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host "--- Finished building: NuGet package ---"-ForegroundColor green

#### Create Build Info File ###############

$build_size = Build-Sizes
$build_info = "# StereoKit v$version Build Information

$build_size"
Set-Content -path 'Tools\BuildInfo.md' -value $build_info
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