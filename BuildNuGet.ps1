$vsExe = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property productPath

function Get-Version {
    $major = Select-String -Path 'StereoKitC\stereokit.h' -Pattern '#define SK_VERSION_MAJOR'
    $minor = Select-String -Path 'StereoKitC\stereokit.h' -Pattern '#define SK_VERSION_MINOR'
    $patch = Select-String -Path 'StereoKitC\stereokit.h' -Pattern '#define SK_VERSION_PATCH'

    Write-Output "v$major.$minor.$patch"
}
function Replace-In-File {
    param($file, $text, $with)

    ((Get-Content -path $file -Raw) -replace $text,$with) | Set-Content -path $file
}
function Clean {
    & $vsExe 'StereoKit.sln' '/Clean' | Out-Null
}
function Build {
    param([parameter(Mandatory)][string] $mode)
    & $vsExe 'StereoKit.sln' '/Build' $mode '/Project' 'StereoKit' | Out-Null
}

Write-Output 'Beginning a full build!'
Get-Version

#Clean
Write-Output 'Cleaned'

#Build -mode "Release|ARM64"
Write-Output "Finished building: ARM64"

$packageOff = '<GeneratePackageOnBuild>false</GeneratePackageOnBuild>'
$packageOn  = '<GeneratePackageOnBuild>true</GeneratePackageOnBuild>'
#Replace-In-File -file 'StereoKit\StereoKit.csproj' -text $packageOff -with $packageOn
#Build -mode "Release|X64"
#Replace-In-File -text $packageOn -with $packageOff
Write-Output "Finished building: X64"