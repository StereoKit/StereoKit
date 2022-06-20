param(
    [ValidateSet('x64','ARM','ARM64')]
    [string]$arch = 'x64',
    [ValidateSet('Win32','UWP')]
    [string]$plat = 'UWP',
    [ValidateSet('Release','Debug')]
    [string]$config = 'Release',
    [string]$gitCache = 'C:\Temp\StereoKitBuild',
    [string]$libOut = '..\StereoKitC\lib'
)

function Get-LineNumber { return $MyInvocation.ScriptLineNumber }
function Get-ScriptName { return $MyInvocation.ScriptName }
function FormatPath { param([string]$path) 
    $archplat = $script:arch
    if ($script:plat -eq 'UWP') {
        $archplat = "$($script:arch)_UWP"
    }
    return $path.Replace('[arch]',$script:arch).Replace('[config]', $config).Replace('[archplat]', $archplat)
}

######## FolderCopy ########
# For managing copy operations after a build!

class FolderCopy {
    [string]$Source
    [string]$Dest
    [bool]$Recurse
    [System.Collections.ArrayList]$extensions = @()

    FolderCopy([string]$src, [string]$dst, [bool]$recurse, [System.Collections.ArrayList]$exts) {
        $this.Source = $src
        $this.Dest = $dst
        $this.Recurse = $recurse
        $this.Extensions = $exts
    }

    [void] Execute() {
        $sourceFinal = FormatPath($this.Source)
        $destFinal   = "$PSScriptRoot\$script:libOut\$(FormatPath($this.Dest))"
        Write-Host "Copy from $sourceFinal to $destFinal"

        New-Item -ItemType Directory -Path $destFinal -Force
        foreach ($ext in $this.Extensions) {
            if ($this.Recurse) {
                Copy-Item -Path "$sourceFinal*.$ext" -Destination $destFinal -Recurse -Force -Confirm:$false | Write-Host
            } else {
                Copy-Item -Path "$sourceFinal*.$ext" -Destination $destFinal -Force -Confirm:$false | Write-Host
            }
        }
        
        if ($null -eq $this.Extensions -or $this.Extensions.Count -eq 0) {
            if ($this.Recurse) {
                Copy-Item -Path $sourceFinal -Destination $destFinal -Recurse -Force -Confirm:$false | Write-Host
            } else {
                Copy-Item -Path $sourceFinal -Destination $destFinal -Force -Confirm:$false | Write-Host
            }
        }
    }
}

######## Dependency ########
# Represents an individual dependency item

class Dependency {
    [string]$Name
    [string]$Repository
    [string]$ReleasePrefix
    [string]$Version
    [string]$Patch
    [System.Collections.ArrayList]$Copies = @()
    [System.Collections.ArrayList]$CmakeOptions = @()
    [bool]$NeedsBuilt

    Dependency([string]$n, [string]$repo, [string]$prefix, [string]$patch, [System.Collections.ArrayList]$options, [System.Collections.ArrayList]$copies) {
        $this.Name = $n
        $this.Repository = $repo
        $this.ReleasePrefix = $prefix
        $this.Copies = $copies
        $this.Patch = $patch
        $this.CmakeOptions = $options
        $this.NeedsBuilt = $false
    }

    [string] Path() {
        if ($script:plat -eq 'Win32') {return "$PSScriptRoot\$script:libOut\bin\$script:arch\$script:config\$($this.Name).lib"}
        else                          {return "$PSScriptRoot\$script:libOut\bin\$($script:arch)_UWP\$script:config\$($this.Name).lib"}
    }
    [string] Key() {
        return "$($this.Name)_$($script:arch)_$($script:plat)_$($script:config)"
    }
}

######## Dependency List ########

$dependencies = @(
    [Dependency]::new(
        'openxr_loader', 
        'https://github.com/KhronosGroup/OpenXR-SDK.git', 
        'release-', 
        $null, 
        @('-DOPENXR_DEBUG_POSTFIX=""'), 
        @(  [FolderCopy]::new('src\loader\[config]\', "bin\[archplat]\[config]\", $false, @('lib', 'pdb', 'dll') ),
            [FolderCopy]::new('..\include\openxr\', "include\openxr\", $false, @('h')))
    ), 
    [Dependency]::new(
        'reactphysics3d',
        'https://github.com/DanielChappuis/reactphysics3d.git',
        'v',
        'reactphysics.patch',
        $null,
        @(  [FolderCopy]::new('[config]\', "bin\[archplat]\[config]\", $false, @('lib', 'pdb', 'dll') ),
            [FolderCopy]::new('..\include\reactphysics3d\*', "include\reactphysics3d\", $true, $null) )
    )
)

#####################################################
######## Check which Dependencies need built ########
#####################################################

Push-Location -Path $PSScriptRoot

# Get the files that contain data about the current dependencies
$sourceFile = Get-Content -Path '..\xmake.lua'
$depsFile   = ''
if (Test-Path -Path 'deps_current.txt' -PathType Leaf) {
    $depsFile = Get-Content -Path 'deps_current.txt' -Raw
}
# Read a hash table from the file
$dependencyVersions = ConvertFrom-StringData -String $depsFile

# Check each dependency to see if it needs to be built
$dependenciesDirty = $false
foreach($dep in $dependencies) {
    $desiredVersion = $sourceFile | Select-String -Pattern "add_requires\(`"$($dep.Name) (.*?)`"" | %{$_.Matches.Groups[1].Value}
    $currentVersion = $dependencyVersions[$dep.Key()]

    if ($desiredVersion -ne $currentVersion) {
        # Check if the versions don't match
        Write-Host $dep.Name ": $currentVersion -> $desiredVersion"
        $dep.NeedsBuilt = $true
        $dependenciesDirty = $true
    } elseif (!(Test-Path -Path $dep.Path() -PathType Leaf)) {
        # Or check if the file is missing for some reason
        Write-Host "$($dep.Name) library file is missing, building $desiredVersion"
        $dep.NeedsBuilt = $true
        $dependenciesDirty = $true
    } else {
        Write-Host $dep.Name ": $currentVersion"
    }

    $dep.Version = $desiredVersion
}

# Exit early if the dependencies are all good!
if (!$dependenciesDirty) {
    Write-Host 'All dependencies are up-to-date'
    exit 0
}

#############################################
######## Check Visual Studio version ########
#############################################

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
######## Build the Dependencies ########
########################################

foreach($dep in $dependencies) {
    #Skip this item if it doesn't need built
    if (!$dep.NeedsBuilt) {continue}

    # Make sure the repository is cloned and pointing to the right version
    $folderName = "$gitCache\$($dep.Name)"
    if (!(Test-Path -Path $folderName)) {
        & git clone $dep.Repository "$folderName"
    }
    Push-Location -Path $folderName
    & git fetch
    & git clean -fd
    & git reset --hard
    & git checkout "$($dep.ReleasePrefix)$($dep.Version)"

    if ($null -ne $dep.Patch -and $dep.Patch -ne '') {
        Write-Host "Applying patch: $($dep.Patch)"
        & git apply "$PSScriptRoot\$($dep.Patch)"
    }

    # Make a build folder for the current build options
    $buildFolder = "build_$($arch)_$($plat)_$($config)"
    if (!(Test-Path -Path $buildFolder)) {
        New-Item -Path . -Name $buildFolder -ItemType "directory" | Out-Null
    }
    Push-Location -Path $buildFolder

    # Configure build settings
    Write-Host "$($dep.Name): Configuring $arch $plat" -ForegroundColor green
    if ($plat -eq 'UWP') {
        & $cmakeCmd -G $vsGeneratorName -A $arch "-DCMAKE_BUILD_TYPE=$config" $dep.CmakeOptions '-DCMAKE_CXX_FLAGS=/MP' '-DCMAKE_SYSTEM_NAME=WindowsStore' '-DCMAKE_SYSTEM_VERSION=10.0' '-DDYNAMIC_LOADER=OFF' '-Wno-deprecated' '-Wno-dev' ..
    } else {
        & $cmakeCmd -G $vsGeneratorName -A $arch "-DCMAKE_BUILD_TYPE=$config" $dep.CmakeOptions '-DCMAKE_CXX_FLAGS=/MP' '-DDYNAMIC_LOADER=OFF' '-Wno-deprecated' '-Wno-dev' ..
    }
    if ($LASTEXITCODE -ne 0) {
        Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: --- $($dep.Name) config failed! Stopping build! ---" -ForegroundColor red
        exit 1
    }

    # And build it all!
    Write-Host "$($dep.Name): Building $arch $plat" -ForegroundColor green
    & $cmakeCmd --build . --config $config -- /m
    if ($LASTEXITCODE -ne 0) {
        Write-Host "$(Get-ScriptName)($(Get-LineNumber),0): error: --- $($dep.Name) build failed! Stopping build! ---" -ForegroundColor red
        exit 1
    }

    # Copy over the results
    Write-Host "$($dep.Name): Copying results $arch $plat" -ForegroundColor green
    foreach($copy in $dep.Copies) {
        $copy.Execute();
    }

    # Update our tracked version number
    Write-Host "$($dep.Name): Finished $arch $plat" -ForegroundColor green
    $dependencyVersions[$dep.Key()] = $dep.Version

    Pop-Location
    # Remove the build folder to save space
    Remove-Item -Path $buildFolder -Recurse -Force -Confirm:$false
    Pop-Location
}

# Save dependencies and their current versions to file
$dependencyVersions.GetEnumerator() | ForEach-Object { "{0} = {1}" -f $_.Name, $_.Value } | Set-Content 'deps_current.txt'

Pop-Location
