param(
    [switch]$upload = $false,
    [switch]$fast = $false,
    [string]$key = ''
)

$vsExe = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property productPath
$vsExe = [io.path]::ChangeExtension($vsExe, '.com')

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
function Replace-In-File {
    param($file, $text, $with)

    ((Get-Content -path $file) -replace $text,$with) | Set-Content -path $file
}
function Clean {
    & $vsExe 'StereoKit.sln' '/Clean' | Out-Null
}
function Build {
    param([parameter(Mandatory)][string] $mode)
    & $vsExe 'StereoKit.sln' '/Build' $mode '/Project' 'StereoKit' | Write-Host
    return $LASTEXITCODE
}
function Test {
    & $vsExe 'StereoKit.sln' '/Build' 'Debug|X64' '/Project' 'StereoKitDocumenter' | Out-Null
    if ($LASTEXITCODE -ne 0) {
        return $LASTEXITCODE
    }
    & 'cd' 'StereoKitDocumenter/bin/Debug/';
    & '.\StereoKitDocumenter.exe' | Write-Host
    & 'cd' '../../../';
    return $LASTEXITCODE
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

# Notify about our upload flag status 
if ($fast -eq $true -and $upload -eq $true) {
    Write-Host "Let's not upload a fast build, just in case! Try again without the fast flag :)" -ForegroundColor yellow
    exit
}

# Notify about our upload flag status 
if ($upload -eq $false) {
    Write-Host 'Local package build only.'
} else {
    Write-Host 'Will attempt to upload package when finished!'
}
if ($fast -eq $true) {
    Write-Host 'Making a "fast" build, incremental build issues may be present.'
}

# Print version, so we know we're building the right version right away
$version = Get-Version
Write-Host "v$version"

# Ensure the version string for the package matches the StereoKit version
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text '<Version>(.*)</Version>' -with "<Version>$version</Version>"
Replace-In-File -file 'xmake.lua' -text 'set_version(.*)' -with "set_version(`"$version`")"

# Run tests before anything else!
if ($fast -eq $false) {
    Write-Host 'Running tests...'
    if ( Test -ne 0 ) {
        Write-Host '--- Tests failed! Stopping build! ---' -ForegroundColor red
        exit
    }
    Write-Host 'Tests passed!' -ForegroundColor green
} else {
    Write-Host 'Skipping tests for fast build!' -ForegroundColor yellow
}

# Notify of build, and output the version
Write-Host 'Beginning a full build!'

# Clean out the old files, do a full build
Remove-Item 'bin\distribute' -Recurse
if ($fast -eq $false) {
    Clean
}
Write-Host 'Cleaned'

# Do cross platform build code first
Write-Host 'Beginning Android build!'
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

# Linux, via WSL
Write-Host 'Beginning Linux build via WSL!'
if ($fast -eq $false) {
    cmd /c "wsl cd /mnt/c/Data/Repositories/StereoKit ; xmake f -p linux -a x64 -m release ; xmake -r"
} else {
    cmd /c "wsl cd /mnt/c/Data/Repositories/StereoKit ; xmake f -p linux -a x64 -m release ; xmake"
}
if ($LASTEXITCODE -ne 0) {
    Write-Host '--- Linux build failed! Stopping build! ---' -ForegroundColor red
    exit
}

# Build ARM first
$result = Build -mode "Release|ARM64"
if ($result -ne 0) {
    Write-Host '--- ARM64 build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host "Finished building: ARM64" -ForegroundColor green

# Turn on NuGet package generation, build x64, then turn it off again
$packageOff = '<GeneratePackageOnBuild>false</GeneratePackageOnBuild>'
$packageOn  = '<GeneratePackageOnBuild>true</GeneratePackageOnBuild>'
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOff -with $packageOn
$result = Build -mode "Release|X64"
Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOn -with $packageOff
if ($result -ne 0) {
    Write-Host '--- x64 build failed! Stopping build! ---' -ForegroundColor red
    exit
}
Write-Host "Finished building: X64" -ForegroundColor green

if ($upload) {
    $key = Get-Key
    if ($key -ne '') {
        & dotnet nuget push "bin\StereoKit.$version.nupkg" -k $key -s https://api.nuget.org/v3/index.json
    } else {
        Write-Host 'No key, cancelling upload'
    }
}

Write-Host "Done!" -ForegroundColor green