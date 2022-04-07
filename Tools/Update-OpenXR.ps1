Push-Location -Path $PSScriptRoot

function Get-LineNumber { return $MyInvocation.ScriptLineNumber }
function Get-ScriptName { return $MyInvocation.ScriptName }

# Check the OpenXR version installed
$openxrDesired = Select-String -Path "..\xmake.lua" -Pattern 'add_requires\("openxr_loader (.*?)"' | %{$_.Matches.Groups[1].Value}
if (Test-Path -Path oxr_current.txt -PathType Leaf) {
    $openxrCurrent = Get-Content -Path "oxr_current.txt"
} else {
    $openxrCurrent = ""
}

if ($openxrCurrent -ne $openxrDesired) {
    Write-Host "Updating to the correct OpenXR loader version!"
    Write-Host "$openxrCurrent -> $openxrDesired"
} else {
    Write-Host "OpenXR is up-to-date" -ForegroundColor green
    exit
}

# Get the Visual Studio executable for building
$vsWhere        = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vsVersionRange = '[16.0,18.0)'
$vsExe          = & $vsWhere -latest -property productPath -version $vsVersionRange
if (!$vsExe) {
    Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: Valid Visual Studio version not found!" -ForegroundColor red
    exit 
}
$vsYear          = & $vsWhere -latest -property catalog_productLineVersion -version $vsVersionRange
$vsVersion       = & $vsWhere -latest -property catalog_buildVersion -version $vsVersionRange
$vsVersion       = $vsVersion.Split(".")[0]
$vsGeneratorName = "Visual Studio $vsVersion $vsYear"
Write-Host "Using $vsGeneratorName" -ForegroundColor green

# Check for cmake 3.21
if (!(Get-Command 'cmake' -errorAction SilentlyContinue))
{
    Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: Cmake not detected! It is needed to build OpenXR, please install or add to Path!" -ForegroundColor red
    exit
}
$Matches = {}
$cmakeVersion = & cmake --version
$cmakeVersion = [string]$cmakeVersion
$cmakeVersion -match '(?<Major>\d+)\.(?<Minor>\d+)\.(?<Patch>\d+)' | Out-Null
$cmvMajor = $Matches.Major
$cmvMinor = $Matches.Minor
$cmvPatch = $Matches.Patch
if ( $cmvMajor -lt 3 -or
    ($cmvMajor -eq 3 -and $cmvMinor -lt 21)) {
    Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: Cmake version must be greater than 3.21! Found $cmvMajor.$cmvMinor.$cmvPatch. Please update and try again!" -ForegroundColor red
    exit
} else {
    Write-Host "Using cmake version: $cmvMajor.$cmvMinor.$cmvPatch" -ForegroundColor green
}

# This tell VS to build with a partcular 'Release|x64' style mode
function Build {
    param([parameter(Mandatory)][string] $mode)
    & $vsExe 'OPENXR.sln' '/Build' $mode '/Project' 'ALL_BUILD' | Out-Null
    return $LASTEXITCODE
}

# This'll build and copy files for a particular OpenXR configuration
function Build-Config {
    param([parameter(Mandatory)][string] $config, [parameter(Mandatory)][string] $target)

    Write-Host "Beginning build: $config"
    Push-Location -Path $config

    # Build release and debug mode for this configuration
    $result = Build -mode "Release|$target"
    if ($result -ne 0) {
        Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: --- $config build failed! Stopping build! ---" -ForegroundColor red
        exit
    }
    $result = Build -mode "Debug|$target"
    if ($result -ne 0) {
        Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: --- $config debug build failed! Stopping build! ---" -ForegroundColor red
        exit
    }
    Write-Host "Built $config complete!" -ForegroundColor green

    #copy files into the project
    New-Item -ItemType Directory -Path "..\..\..\..\StereoKitC\lib\bin\$config\Release" -Force
    New-Item -ItemType Directory -Path "..\..\..\..\StereoKitC\lib\bin\$config\Debug" -Force
    Copy-Item -Path "src\loader\Release\openxr_loader.lib" -Destination "..\..\..\..\StereoKitC\lib\bin\$config\Release\openxr_loader.lib" -Force -Confirm:$false
    Copy-Item -Path "src\loader\Debug\openxr_loaderd.lib" -Destination "..\..\..\..\StereoKitC\lib\bin\$config\Debug\openxr_loader.lib" -Force -Confirm:$false

    Pop-Location
}

# Clone the repository
& git clone "https://github.com/KhronosGroup/OpenXR-SDK.git"
Push-Location -Path "OpenXR-SDK"
& git checkout release-$openxrDesired

# Create build folders
New-Item -Path . -Name "build" -ItemType "directory" | Out-Null
Push-Location -Path "build"
New-Item -Path . -Name "x64" -ItemType "directory" | Out-Null
New-Item -Path . -Name "x64_UWP" -ItemType "directory" | Out-Null
New-Item -Path . -Name "ARM64" -ItemType "directory" | Out-Null
New-Item -Path . -Name "ARM64_UWP" -ItemType "directory" | Out-Null
New-Item -Path . -Name "ARM_UWP" -ItemType "directory" | Out-Null

# cmake each project configuration
Push-Location -Path "x64"
Write-Host 'Making x64' -ForegroundColor green
& cmake -G $vsGeneratorName -A x64 ../..
Write-Host 'Made x64' -ForegroundColor green
Pop-Location

Push-Location -Path "x64_UWP"
Write-Host 'Making x64_UWP' -ForegroundColor green
& cmake -G $vsGeneratorName -A x64 "-DCMAKE_SYSTEM_NAME=WindowsStore" "-DCMAKE_SYSTEM_VERSION=10.0" "-DDYNAMIC_LOADER=OFF" ../..
Write-Host 'Made x64_UWP' -ForegroundColor green
Pop-Location

Push-Location -Path "ARM64"
Write-Host 'Making ARM64' -ForegroundColor green
& cmake -G $vsGeneratorName -A ARM64 ../..
Write-Host 'Made ARM64' -ForegroundColor green
Pop-Location

Push-Location -Path "ARM64_UWP"
Write-Host 'Making ARM64_UWP' -ForegroundColor green
& cmake -G $vsGeneratorName -A ARM64 "-DCMAKE_SYSTEM_NAME=WindowsStore" "-DCMAKE_SYSTEM_VERSION=10.0" "-DDYNAMIC_LOADER=OFF" ../..
Write-Host 'Made ARM64_UWP' -ForegroundColor green
Pop-Location

Push-Location -Path "ARM_UWP"
Write-Host 'Making ARM_UWP' -ForegroundColor green
& cmake -G $vsGeneratorName -A ARM "-DCMAKE_SYSTEM_NAME=WindowsStore" "-DCMAKE_SYSTEM_VERSION=10.0" "-DDYNAMIC_LOADER=OFF" ../..
Write-Host 'Made ARM_UWP' -ForegroundColor green
Pop-Location

# Now we'll build each project
Build-Config -config "x64" -target "x64"
Build-Config -config "x64_UWP" -target "x64"
Build-Config -config "ARM64" -target "ARM64"
Build-Config -config "ARM64_UWP" -target "ARM64"
Build-Config -config "ARM_UWP" -target "ARM"

Pop-Location

# Copy include files
Write-Host 'Copying headers'
Copy-Item -Path "include\openxr\*.h" -Destination "..\..\StereoKitC\lib\include\openxr" -Force -Confirm:$false
Write-Host 'Headers copied!' -ForegroundColor green

# Clean up and delete files!
Pop-Location
Remove-Item -Path "OpenXR-SDK" -Recurse -Force -Confirm:$false

# Mark the current OpenXR version down
Set-Content -Path 'oxr_current.txt' -Value $openxrDesired

Pop-Location

# We'll tack on a React Physics build here as well. This may mean
# more builds of react physics than strictly necessary, but we do
# tap into the OXR build-if-necessary logic.
Write-Host 'Building ReactPhysics3D too!'

# Clone the repository
& git clone https://github.com/DanielChappuis/reactphysics3d.git
Push-Location -Path 'reactphysics3d'
& git checkout 4bbbaa7c6e92942734eec696e23a2fad1f1cb8a1 # v0.9

# Replace the include files
Remove-Item -Path '../../StereoKitC/lib/include/reactphysics3d' -Recurse -Force -Confirm:$false
New-Item -Path . -Name '../../StereoKitC/lib/include/reactphysics3d' -ItemType 'directory' | Out-Null
Copy-Item -Path "include\reactphysics3d\*" -Destination "../../StereoKitC/lib/include/reactphysics3d" -Recurse -Force -Confirm:$false

# Build x64 Release/Debug, ARM64 Release/Debug and ARM Release/Debug with Visual Studio
New-Item -Path . -Name 'Release_x64' -ItemType 'directory' -Force | Out-Null
Push-Location -Path 'Release_x64'
& cmake -G "$vsGeneratorName" -A x64 -DCMAKE_BUILD_TYPE=Release ../ -Wno-dev
& cmake --build . --config Release
Copy-Item -Path "Release/reactphysics3d.lib" -Destination "../../../StereoKitC/lib/bin/x64/Release/" -Force -Confirm:$false
Pop-Location

New-Item -Path . -Name 'Debug_x64' -ItemType 'directory' | Out-Null
Push-Location -Path 'Debug_x64'
& cmake -G $vsGeneratorName -A x64 -DCMAKE_BUILD_TYPE=Debug ../ -Wno-dev
& cmake --build .
Copy-Item -Path "Debug/reactphysics3d.lib" -Destination "../../../StereoKitC/lib/bin/x64/Debug/" -Force -Confirm:$false
Copy-Item -Path "Debug/reactphysics3d.pdb" -Destination "../../../StereoKitC/lib/bin/x64/Debug/" -Force -Confirm:$false
Pop-Location

New-Item -Path . -Name 'Release_ARM64' -ItemType 'directory' | Out-Null
Push-Location -Path 'Release_ARM64'
& cmake -G $vsGeneratorName -A ARM64 -DCMAKE_BUILD_TYPE=Release ../ -Wno-dev
& cmake --build . --config Release 
Copy-Item -Path "Release/reactphysics3d.lib" -Destination "../../../StereoKitC/lib/bin/ARM64/Release/" -Force -Confirm:$false
Pop-Location

New-Item -Path . -Name 'Debug_ARM64' -ItemType 'directory' | Out-Null
Push-Location -Path 'Debug_ARM64'
& cmake -G $vsGeneratorName -A ARM64 -DCMAKE_BUILD_TYPE=Debug ../ -Wno-dev
& cmake --build .
Copy-Item -Path "Debug/reactphysics3d.lib" -Destination "../../../StereoKitC/lib/bin/ARM64/Debug/" -Force -Confirm:$false
Copy-Item -Path "Debug/reactphysics3d.pdb" -Destination "../../../StereoKitC/lib/bin/ARM64/Debug/" -Force -Confirm:$false
Pop-Location

New-Item -Path . -Name 'Release_ARM' -ItemType 'directory' | Out-Null
Push-Location -Path 'Release_ARM'
& cmake -G $vsGeneratorName -A ARM -DCMAKE_BUILD_TYPE=Release ../ -Wno-dev
& cmake --build . --config Release 
Copy-Item -Path "Release/reactphysics3d.lib" -Destination "../../../StereoKitC/lib/bin/ARM/Release/" -Force -Confirm:$false
Pop-Location

New-Item -Path . -Name 'Debug_ARM' -ItemType 'directory' | Out-Null
Push-Location -Path 'Debug_ARM'
& cmake -G $vsGeneratorName -A ARM -DCMAKE_BUILD_TYPE=Debug ../ -Wno-dev
& cmake --build .
Copy-Item -Path "Debug/reactphysics3d.lib" -Destination "../../../StereoKitC/lib/bin/ARM/Debug/" -Force -Confirm:$false
Copy-Item -Path "Debug/reactphysics3d.pdb" -Destination "../../../StereoKitC/lib/bin/ARM/Debug/" -Force -Confirm:$false
Pop-Location

Pop-Location

Remove-Item -Path "reactphysics3d" -Recurse -Force -Confirm:$false