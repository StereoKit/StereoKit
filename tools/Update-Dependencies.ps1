param(
    [ValidateSet('x86','x64','ARM','ARM64')]
    [string]$arch = 'x64',
    [ValidateSet('Win32','UWP')]
    [string]$plat = 'UWP',
    [ValidateSet('Release','Debug')]
    [string]$config = 'Release',
    [string]$gitCache = 'C:\Temp\StereoKitBuild',
    [string]$libOut = '..\StereoKitC\lib',
    [string]$onlyDep = ''
)

function Get-LineNumber { return $MyInvocation.ScriptLineNumber }
function Get-ScriptName { return $MyInvocation.ScriptName }
function FormatPath { param([string]$path) 
    $archplat = $script:arch
    if ($script:plat -eq 'UWP') {
        $archplat = "$($script:arch)_UWP"
    }
    return $path.Replace('[arch]',$script:arch).Replace('[config]', $config).Replace('[archplat]', $archplat).Replace('[libfolder]', $script:libOut)
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
        $destFinal   = "$PSScriptRoot\$(FormatPath($this.Dest))"
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
    [string]$Tag
    [string]$Version
    [string]$VerifyFile
    [System.Collections.ArrayList]$Copies = @()
    [System.Collections.ArrayList]$CmakeOptions = @()
    [bool]$NeedsBuilt

    Dependency([string]$n, [string]$repo, [System.Collections.ArrayList]$options, [System.Collections.ArrayList]$copies, [string]$verifyFile) {
        $this.Name = $n
        $this.Repository = $repo
        $this.Tag = ""
        $this.Copies = $copies
        $this.CmakeOptions = $options
        $this.NeedsBuilt = $false
        $this.VerifyFile = $verifyFile
    }

    [string] Path() {
        if ($this.VerifyFile -ne '' -and $null -ne $this.VerifyFile) {return FormatPath($this.VerifyFile)}

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
        @('-DOPENXR_DEBUG_POSTFIX=""'), 
        @(  [FolderCopy]::new('src\loader\[config]\', "[libfolder]\bin\[archplat]\[config]\", $false, @('lib', 'pdb', 'dll') ),
            [FolderCopy]::new('..\include\openxr\', "[libfolder]\include\openxr\", $false, @('h'))),
        $null
    )
    [Dependency]::new(
        'sk_gpu', 
        'https://github.com/StereoKit/sk_gpu/releases/download/v[version]/sk_gpu.v[version].zip',
        @(), 
        @(  [FolderCopy]::new('..\tools\*', 'skshaderc\', $true, $null),
            [FolderCopy]::new('..\src\*', '[libfolder]\include\sk_gpu\', $false, @('h'))),
        'skshaderc\win32_x64\skshaderc.exe'
    )
    [Dependency]::new(
        'meshoptimizer',
        'https://github.com/zeux/meshoptimizer.git',
        $null,
        @(  [FolderCopy]::new('[config]\', "[libfolder]\bin\[archplat]\[config]\", $false, @('lib', 'pdb', 'dll') ),
            [FolderCopy]::new('..\src\*', "[libfolder]\include\meshoptimizer\", $false, @('h')) ),
        $null
    )
    [Dependency]::new(
        'basis_universal',
        'https://github.com/BinomialLLC/basis_universal.git',
        $null,
        @(  [FolderCopy]::new('[config]\', "[libfolder]\bin\[archplat]\[config]\", $false, @('lib', 'pdb', 'dll') ),
            [FolderCopy]::new('..\transcoder\*', "[libfolder]\include\basisu\", $false, @('h', 'cpp', 'inc')),
            [FolderCopy]::new('..\zstd\*', "[libfolder]\include\zstd\", $false, @('h', 'c')) ),
        "[libfolder]\include\basisu\basisu_transcoder.h"
    )
)

#####################################################
######## Check which Dependencies need built ########
#####################################################

Push-Location -Path $PSScriptRoot

# Get the files that contain data about the current dependencies
$sourceFile = Get-Content -Path '..\CMakeLists.txt' -Raw
$depsFile   = ''
if (Test-Path -Path 'deps_current.txt' -PathType Leaf) {
    $depsFile = Get-Content -Path 'deps_current.txt' -Raw
}
# Read a hash table from the file
$dependencyVersions = ConvertFrom-StringData -String $depsFile

# Check each dependency to see if it needs to be built
$dependenciesDirty = $false
foreach($dep in $dependencies) { # NAME\s*$($dep.Name)\s*#\s*([\d.]*)
    if ($onlyDep -ne '' -and $dep.Name -ne $onlyDep) {continue}

    $tag            = $sourceFile | Select-String -Pattern "(?s)NAME\s*$($dep.Name).*?GIT_TAG\s*(\w*)" | %{$_.Matches.Groups[1].Value}
    $desiredVersion = $sourceFile | Select-String -Pattern "NAME\s*$($dep.Name)\s*#\s*([\d.]*)" | %{$_.Matches.Groups[1].Value}
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
    $dep.Tag     = $tag;
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
$vsYear          = & $vsWhere -latest -prerelease -property catalog_productLineVersion -version $vsVersionRange
$vsVersion       = & $vsWhere -latest -prerelease -property catalog_buildVersion -version $vsVersionRange
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

    # Check if the repository ends with .git or .zip
    $folderName = "$gitCache\$($dep.Name)"
    if ($dep.Repository.EndsWith('.git')) {
        # Make sure the repository is cloned and pointing to the right version
        if (!(Test-Path -Path $folderName)) {
            & git clone $dep.Repository "$folderName"
        }
        Push-Location -Path $folderName
        & git fetch
        & git clean -fd
        & git reset --hard
        & git checkout "$($dep.Tag)"

        Write-Host "Checked out $($dep.Name) at $($dep.Tag)"
    } else {
        $zipName = "$folderName\$($dep.Name).$(($dep.Version)).zip"

        if (!(Test-Path -Path $zipName)) {
            Write-Host "Downloading $($dep.Name) $($dep.Version)"
            # if the folder exists, we need to delete it
            if (Test-Path -Path $folderName) {
                Remove-Item -Path $folderName -Recurse -Force -Confirm:$false
            }
            New-Item -Path $folderName -ItemType "directory" | Out-Null

            # Download the zip file and extract it
            $url = $dep.Repository.Replace('[version]', $dep.Version)
            Invoke-WebRequest -Uri $url -OutFile $zipName
            Expand-Archive -Path $zipName -DestinationPath $folderName -Force

            # Touch all the files to make sure they have the current time,
            # which is apparently not guaranteed when unzipping.
            foreach ($file in (Get-ChildItem -Path $folderName -Recurse -File)) {
                $currentTime = Get-Date
                $file.LastWriteTime  = $currentTime
                $file.CreationTime   = $currentTime
                $file.LastAccessTime = $currentTime
            }

            Write-Host "Downloaded finished"
        }
        Push-Location -Path $folderName
    } 

    # Make a build folder for the current build options
    $buildFolder = "build_$($arch)_$($plat)_$($config)"
    if (!(Test-Path -Path $buildFolder)) {
        New-Item -Path . -Name $buildFolder -ItemType "directory" | Out-Null
    }
    Push-Location -Path $buildFolder

    # Configure build settings
    Write-Host "$($dep.Name): Configuring $arch $plat" -ForegroundColor green
    $vsArch = $arch
    if ($vsArch -eq 'x86') {
        $vsArch = 'Win32'
    }
    if ($plat -eq 'UWP') {
        & $cmakeCmd -G $vsGeneratorName -A $vsArch "-DCMAKE_BUILD_TYPE=$config" $dep.CmakeOptions '-DCMAKE_CXX_FLAGS=/MP' '-DCMAKE_SYSTEM_NAME=WindowsStore' '-DCMAKE_SYSTEM_VERSION=10.0' '-DDYNAMIC_LOADER=OFF' '-Wno-deprecated' '-Wno-dev' ..
    } else {
        & $cmakeCmd -G $vsGeneratorName -A $vsArch "-DCMAKE_BUILD_TYPE=$config" $dep.CmakeOptions '-DCMAKE_CXX_FLAGS=/MP' '-DDYNAMIC_LOADER=OFF' '-Wno-deprecated' '-Wno-dev' ..
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
