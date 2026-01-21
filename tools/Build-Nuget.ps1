param(
    [switch]$upload = $false,
    [switch]$noTest = $false,
    [switch]$noBuild = $false,
    [string]$key = '',
    [string]$saveKey = '',
    [switch]$noLinux = $false,
    [switch]$noWin32 = $false,
    [switch]$noAndroid = $false
)

function Get-LineNumber { return $MyInvocation.ScriptLineNumber }
function Get-ScriptName { return $MyInvocation.ScriptName }

# In case we only want to build a subset of the package
$buildWindows    = -not $noWin32 -and -not $noBuild
$buildLinux      = -not $noLinux -and -not $noBuild
$buildAndroid    = -not $noAndroid -and -not $noBuild

$clean = $true -and -not $noBuild

###########################################
## Functions                             ##
###########################################

function Build-Preset {
    param($preset, $presetName, [switch]$wsl = $false)

    Write-Host "--- Beginning build: $presetName ---" -ForegroundColor green
    if ($wsl -eq $true) { 
        & wsl cmake --preset $preset
        & wsl cmake --build --preset $preset
    } else {
        & cmake --preset $preset
        & cmake --build --preset $preset
    }
    if ($LASTEXITCODE -ne 0) {
        Write-Host '--- $presetName build failed! Stopping build! ---' -ForegroundColor red
        Pop-Location
        exit
    }
    Write-Host "--- Finished building: $presetName ---" -ForegroundColor green
}

###########################################
## Main                                  ##
###########################################

# Notify about our upload flag status
Write-Host 'Building... ' -NoNewline
if ($upload) { Write-Host 'AND UPLOADING!' -ForegroundColor Green }
else         { Write-Host 'local only.'    -ForegroundColor White }

# Switch to the right folder
Push-Location -Path "$PSScriptRoot\.."

#### Update Version #######################

$version = & $PSScriptRoot\Get-Version.ps1
# Ensure all versions match the source of truth
& $PSScriptRoot\Set-Version.ps1 -major $version.major -minor $version.minor -patch $version.patch -pre $version.pre

# Notify of build, and output the version
Write-Host @"
   _____ _                      _  __  _   
  / ____| |                    | |/ ( ) |  
 | (___ | |_ ___ _ __ ___  ___ |   / _| |_ 
  \___ \| __/ _ \  __/ _ \/   \|  < | | __|
  ____) | ||  __/ | |  __/ ( ) |   \| | |_ 
 |_____/ \__\___|_|  \___|\___/|_|\_\_|\__| 
"@ -NoNewline -ForegroundColor White
Write-Host "v$($version.str)`n" -ForegroundColor Cyan

#### Clean Project ########################

# Clean out the old files, do a full build
if ($clean) {
    Write-Host 'Cleaning old files...'
    if (Test-Path 'bin\distribute') {
        Remove-Item 'bin\distribute' -Recurse
    }
    if (Test-Path 'bin\intermediate\cmake') {
        Remove-Item 'bin\intermediate\cmake' -Recurse -Force
    }
    Write-Host 'Cleaned'
} else {
    Write-Host 'Skipping clean'
}

#### Build Windows ########################

if ($buildWindows) {
    Write-Host @"
__      __          _               
\ \    / ( )_ _  __| |_____ __ _____
 \ \/\/ /| | ' \/ _  / _ \ V  V (_-<
  \_/\_/ |_|_||_\__ _\___/\_/\_//__/

"@ -ForegroundColor White

    Build-Preset -preset Win32_x64_Release -presetName 'Win32 x64'
    Build-Preset -preset Win32_Arm64_Release -presetName 'Win32 Arm64'
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
    Build-Preset -preset Linux_x64_Release -presetName 'Linux x64' -wsl
}

#### Build Android ########################

if ($buildAndroid) {
    Write-Host @"
    _           _              _ 
   /_\  _ _  __| |_ _ ___( )__| |
  / _ \| ' \/ _' | '_/ _ \ / _' |
 /_/ \_\_||_\__,_|_| \___/_\__,_|
                      
"@ -ForegroundColor White

    Build-Preset -preset Android_x64_Release -presetName 'Android x64'
    Build-Preset -preset Android_Arm64_Release -presetName 'Android arm64-v8a'
}

#### Run tests ########################

# Run tests!
if ($noTest -eq $false) {
    & $PSScriptRoot/Run-Tests.ps1
    if ($LASTEXITCODE -ne 0) {
        Write-Host '--- Tests failed! Stopping build! ---' -ForegroundColor red
        Pop-Location
        exit
    }
    Write-Host 'Tests passed!' -ForegroundColor green
} else {
    Write-Host "`nSkipping tests!" -ForegroundColor yellow
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
$packageOff  = '<GeneratePackageOnBuild>false</GeneratePackageOnBuild>'
$packageOn   = '<GeneratePackageOnBuild>true</GeneratePackageOnBuild>'
$packageFile = "$PSScriptRoot/../StereoKit/StereoKit.csproj"
$partialBuild= '-p:SKIgnoreMissingBinaries=true'
if ($upload) {$partialBuild = ''}
[System.IO.File]::WriteAllText($packageFile, ((Get-Content -Path $packageFile -raw) -replace $packageOff, $packageOn))
& dotnet build StereoKit\StereoKit.csproj $partialBuild -c Release
[System.IO.File]::WriteAllText($packageFile, ((Get-Content -Path $packageFile -raw) -replace $packageOn, $packageOff))
if ($LASTEXITCODE -ne 0) {
    Write-Host '--- NuGet build failed! Stopping build! ---' -ForegroundColor red
    Pop-Location
    exit
}
Write-Host "--- Finished building: NuGet package ---"-ForegroundColor green

#### Create Build Info File ###############

& $PSScriptRoot\Make-Bloat-Report.ps1

#### Upload NuGet Package #################

if ($upload) {
    # Figure out what key to use when uploading
    if ($saveKey -ne '') {
        # User wants us to cache this key in a file for reuse later
        $key = $saveKey
        Set-Content -path '.nugetkey' -value $key.Trim()
    } elseif ($key -ne '') {
        # User provided a key as an argument
    } elseif (Test-Path '.nugetkey') {
        # No user key, check for a cached key
        $key = Get-Content -path '.nugetkey' -Raw
    } else {
        # No key, ask the user for one
        $key = Read-Host "Please enter the NuGet API key, or n to cancel"
        if ($key -eq 'n') {
            $key = ''
        }
    }

    # Upload with our key!
    if ($key -ne '') {
        $key = $key.Trim()
        & dotnet nuget push "$PSScriptRoot/../bin/StereoKit.$($version.str).nupkg" -k $key -s https://api.nuget.org/v3/index.json
    } else {
        Write-Host 'No key, cancelling upload'
    }
}

Write-Host "Done!" -ForegroundColor green

Pop-Location
