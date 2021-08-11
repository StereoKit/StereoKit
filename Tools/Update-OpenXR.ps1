Push-Location -Path $PSScriptRoot

# Check the version installed
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
    Write-Host "OpenXR is up-to-date"
    exit
}

# Get the Visual Studio executable for building
$vsExe = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property productPath

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
        Write-Host "--- $config build failed! Stopping build! ---" -ForegroundColor red
        exit
    }
    $result = Build -mode "Debug|$target"
    if ($result -ne 0) {
        Write-Host "--- $config debug build failed! Stopping build! ---" -ForegroundColor red
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
& cmake -G "Visual Studio 16 2019" -A x64 ../..
Write-Host 'Made x64' -ForegroundColor green
Pop-Location

Push-Location -Path "x64_UWP"
Write-Host 'Making x64_UWP' -ForegroundColor green
& cmake -G "Visual Studio 16 2019" -A x64 "-DCMAKE_SYSTEM_NAME=WindowsStore" "-DCMAKE_SYSTEM_VERSION=10.0" "-DDYNAMIC_LOADER=OFF" ../..
Write-Host 'Made x64_UWP' -ForegroundColor green
Pop-Location

Push-Location -Path "ARM64"
Write-Host 'Making ARM64' -ForegroundColor green
& cmake -G "Visual Studio 16 2019" -A ARM64 ../..
Write-Host 'Made ARM64' -ForegroundColor green
Pop-Location

Push-Location -Path "ARM64_UWP"
Write-Host 'Making ARM64_UWP' -ForegroundColor green
& cmake -G "Visual Studio 16 2019" -A ARM64 "-DCMAKE_SYSTEM_NAME=WindowsStore" "-DCMAKE_SYSTEM_VERSION=10.0" "-DDYNAMIC_LOADER=OFF" ../..
Write-Host 'Made ARM64_UWP' -ForegroundColor green
Pop-Location

Push-Location -Path "ARM_UWP"
Write-Host 'Making ARM_UWP' -ForegroundColor green
& cmake -G "Visual Studio 16 2019" -A ARM "-DCMAKE_SYSTEM_NAME=WindowsStore" "-DCMAKE_SYSTEM_VERSION=10.0" "-DDYNAMIC_LOADER=OFF" ../..
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
C:update_physics_win.bat