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

    # Build Win32 first
    Write-Host "--- Beginning build: Win32 x64 ---" -ForegroundColor green
    & cmake --workflow --preset Win32x64Release
    if ($LASTEXITCODE -ne 0) {
        Write-Host '--- Win32 x64 build failed! Stopping build! ---' -ForegroundColor red
        Pop-Location
        exit
    }
    Write-Host "--- Finished building: Win32 x64 ---" -ForegroundColor green

    # Build UWP next
    if ($buildWindowsUWP) {
        Write-Host "--- Beginning build: UWP x64 ---" -ForegroundColor green
        & cmake --workflow --preset Uwpx64Release
        if ($LASTEXITCODE -ne 0) {
            Write-Host '--- UWP x64 build failed! Stopping build! ---' -ForegroundColor red
            Pop-Location
            exit
        }
        Write-Host "--- Finished building: UWP x64 ---" -ForegroundColor green
        Write-Host "--- Beginning build: UWP ARM64 ---" -ForegroundColor green
        & cmake --workflow --preset UwpArm32Release
        if ($LASTEXITCODE -ne 0) {
            Write-Host '--- UWP ARM64 build failed! Stopping build! ---' -ForegroundColor red
            Pop-Location
            exit
        }
        Write-Host "--- Finished building: UWP ARM64 ---" -ForegroundColor green
        Write-Host "--- Beginning build: UWP ARM ---" -ForegroundColor green
        & cmake --workflow --preset UwpArm64Release
        if ($LASTEXITCODE -ne 0) {
            Write-Host '--- UWP ARM build failed! Stopping build! ---' -ForegroundColor red
            Pop-Location
            exit
        }
        Write-Host "--- Finished building: UWP ARM ---" -ForegroundColor green
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

    # Find the correct WSL folder
    $linux_folder = ''+$PSScriptRoot
    $linux_folder = $linux_folder.replace('\', '/')
    $linux_folder = $linux_folder.replace(':', '')
    $linux_folder = '/mnt/'+$linux_folder
    Write-Output $linux_folder

    # Linux, via WSL
    #Write-Host '--- Beginning WSL build: Linux ARM64 ---' -ForegroundColor green
    #& wsl cmake --workflow --preset LinuxArm64Release
    #if ($LASTEXITCODE -ne 0) {
    #    Write-Host '--- Linux build failed! Stopping build! ---' -ForegroundColor red
    #    Pop-Location
    #    exit
    #}
    #Write-Host '--- Finished building: Linux ARM64 ---' -ForegroundColor green

    Write-Host '--- Beginning WSL build: Linux x64 ---' -ForegroundColor green
    & wsl cmake --workflow --preset Linuxx64Release
    if ($LASTEXITCODE -ne 0) {
        Write-Host '--- Linux build failed! Stopping build! ---' -ForegroundColor red
        Pop-Location
        exit
    }
    Write-Host '--- Finished building: Linux x64 ---' -ForegroundColor green
}

#### Build Android ########################

if ($buildAndroid) {
    Write-Host @"

    _           _              _ 
   /_\  _ _  __| |_ _ ___( )__| |
  / _ \| ' \/ _' | '_/ _ \ / _' |
 /_/ \_\_||_\__,_|_| \___/_\__,_|
                      
"@ -ForegroundColor White

    Write-Host '--- Beginning build: Android arm64-v8a' -ForegroundColor green
    & cmake --workflow --preset AndroidArm64Release
    if ($LASTEXITCODE -ne 0) {
        Write-Host '--- Android build failed! Stopping build! ---' -ForegroundColor red
        Pop-Location
        exit
    }
    Write-Host '--- Finished building: Android arm64-v8a ---' -ForegroundColor green
}

#### Run tests ########################

# Run tests!
if ($fast -eq $false -and $skipTest -eq $false) {
    Write-Host "`nRunning Tests!"
    # build a non-nuget version of C#'s StereoKit.dll for testing
    & dotnet build StereoKit\StereoKit.csproj -c Release -p:SKIgnoreMissingBinaries=true
    if ($LASTEXITCODE -ne 0 ) {
        Write-Host '--- Tests failed! Stopping build! ---' -ForegroundColor red
        Pop-Location
        exit
    }

    # And run tests for a few platforms
    if ($buildWindows) {
        & dotnet run -c Release --project Examples\StereoKitTest\StereoKitTest.csproj -- -test -screenfolder "$PSScriptRoot/Screenshots/" | Write-Host
        if ($LASTEXITCODE -ne 0 ) {
            Write-Host '--- Tests failed! Stopping build! ---' -ForegroundColor red
            Pop-Location
            exit
        }
    }
    if ($buildLinux) {
        & wsl LIBGL_ALWAYS_SOFTWARE=1 dotnet run -c Release --project Examples/StereoKitTest/StereoKitTest.csproj -- -test -noscreens | Write-Host
        if ($LASTEXITCODE -ne 0 ) {
            Write-Host '--- Tests failed! Stopping build! ---' -ForegroundColor red
            Pop-Location
            exit
        }
    }
    Write-Host 'Tests passed!' -ForegroundColor green
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