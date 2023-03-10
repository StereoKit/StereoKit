param(
    [ValidateSet('x86','x64','ARM','ARM64')]
    [string]$arch = 'ARM64',
    [ValidateSet('Win32','UWP','Linux','Android')]
    [string]$plat = 'Android',
    [ValidateSet('Release','Debug')]
    [string]$config = 'Release'
)

$build_folder = "$($plat)_$($arch)_$($config)"

function Get-LineNumber { return $MyInvocation.ScriptLineNumber }
function Get-ScriptName { return $MyInvocation.ScriptName }

#############################################
######## Check Visual Studio version ########
#############################################

if ($plat -eq 'UWP' -or $plat -eq 'Win32') {
    # Get the Visual Studio executable for building
    $vsWhere        = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
    $vsVersionRange = '[16.0,18.0)'
    $vsExe          = & $vsWhere -latest -prerelease -property productPath -version $vsVersionRange
    if (!$vsExe) {
        Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: Valid Visual Studio version not found!" -ForegroundColor red
        exit 
    }
    $vsYear          = & $vsWhere -latest -property catalog_productLineVersion -version $vsVersionRange
    $vsVersion       = & $vsWhere -latest -property catalog_buildVersion -version $vsVersionRange
    $vsVersion       = $vsVersion.Split(".")[0]
    $vsGeneratorName = "Visual Studio $vsVersion $vsYear"
    Write-Host "Using $vsGeneratorName" -ForegroundColor green
}

#####################################
######## Check CMake version ########
#####################################

# Check for cmake 3.21
$cmakeCmd = 'cmake'
if (!(Get-Command "$cmakeCmd" -errorAction SilentlyContinue))
{
    # cmake is not in PATH, ask vswhere for a cmake version
    $cmakeCmd = & $vsWhere -latest -prerelease -version $vsVersionRange -requires Microsoft.VisualStudio.Component.VC.CMake.Project -find 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if ($cmakeCmd -eq "" -or $null -eq $cmakeCmd -or !(Get-Command "$cmakeCmd" -errorAction SilentlyContinue))
    {
        # No cmake available, error out and point users to cmake's website 
        Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: Cmake not detected! It is needed to build dependencies, please install or add to Path!" -ForegroundColor red
        Start-Process 'https://cmake.org/'
        exit 1
    }
}
$Matches = {}
$cmakeVersion = & $cmakeCmd --version
$cmakeVersion = [string]$cmakeVersion
$cmakeVersion -match '(?<Major>\d+)\.(?<Minor>\d+)\.(?<Patch>\d+)' | Out-Null
$cmvMajor = $Matches.Major
$cmvMinor = $Matches.Minor
$cmvPatch = $Matches.Patch
if ( $cmvMajor -lt 3 -or
    ($cmvMajor -eq 3 -and $cmvMinor -lt 21)) {
    Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: Cmake version must be greater than 3.21! Found $cmvMajor.$cmvMinor.$cmvPatch. Please update and try again!" -ForegroundColor red
    Start-Process 'https://cmake.org/'
    exit 1
} else {
    Write-Host "Using cmake version: $cmvMajor.$cmvMinor.$cmvPatch" -ForegroundColor green
}

########################################

Push-Location '..'
if (!(Test-Path -Path 'build_cmake')) {
    New-Item -Path . -Name 'build_cmake' -ItemType "directory" | Out-Null
}
Push-Location 'build_cmake'
if (!(Test-Path -Path $build_folder)) {
    New-Item -Path . -Name $build_folder -ItemType "directory" | Out-Null
}
Push-Location $build_folder

Write-Host "Configuring $arch $plat" -ForegroundColor green
$vsArch = $arch
if ($vsArch -eq 'x86') {
    $vsArch = 'Win32'
}
if ($plat -eq 'UWP') {
    & $cmakeCmd -G $vsGeneratorName -A $vsArch "-DCMAKE_BUILD_TYPE=$config" '-DCMAKE_CXX_FLAGS=/MP' '-DCMAKE_SYSTEM_NAME=WindowsStore' '-DCMAKE_SYSTEM_VERSION=10.0' '-DDYNAMIC_LOADER=ON' '-Wno-deprecated' '-Wno-dev' ../..
} elseif ($plat -eq 'Android') {
    # https://developer.android.com/ndk/guides/cmake

    if ($null -eq $Env:NDK) {
        Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: NDK not found! Make sure the NDK environment variable is defined." -ForegroundColor red
        exit 1
    }
    Write-Host "Using NDK: $Env:NDK" -ForegroundColor green

    $androidArch = $arch
    if     ($arch -eq 'x64')   { $androidArch = 'x86_64'}
    elseif ($arch -eq 'ARM64') { $androidArch = 'arm64-v8a' }

    & $cmakeCmd -G Ninja `
        "-DCMAKE_BUILD_TYPE=$config" `
        "-DANDROID_ABI=$androidArch" `
        "-DCMAKE_ANDROID_ARCH_ABI=$androidArch" `
        "-DCMAKE_TOOLCHAIN_FILE=$Env:NDK/build/cmake/android.toolchain.cmake" `
        "-DANDROID_NDK=$Env:NDK" `
        "-DCMAKE_ANDROID_NDK=$Env:NDK" `
        '-DANDROID_PLATFORM=android-21' `
        '-DCMAKE_SYSTEM_VERSION=21' `
        '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON' `
        '-DCMAKE_SYSTEM_NAME=Android' `
        '-DCMAKE_MAKE_PROGRAM=C:/Users/progr/AppData/Local/Android/Sdk/cmake/3.18.1/bin/ninja.exe' `
        '-DDYNAMIC_LOADER=ON' `
        '-Wno-deprecated' `
        '-Wno-dev' `
        ../..

} else {
    & $cmakeCmd -G $vsGeneratorName -A $vsArch "-DCMAKE_BUILD_TYPE=$config" '-DCMAKE_CXX_FLAGS=/MP' '-DDYNAMIC_LOADER=ON' '-Wno-deprecated' '-Wno-dev' ../..
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: Cmake configuration failed! Error code: $LASTEXITCODE" -ForegroundColor red
    exit 1
}

Write-Host "Building $arch $plat" -ForegroundColor green
& $cmakeCmd --build . --config $config --

Pop-Location
Pop-Location
Pop-Location