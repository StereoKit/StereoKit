param(
    [switch]$noWindows = $false,
    [switch]$noLinux = $false
)

Write-Host "`nRunning Tests!"
# build a non-nuget version of C#'s StereoKit.dll for testing
& dotnet build "$PSScriptRoot\..\StereoKit\StereoKit.csproj" -c Release -p:SKIgnoreMissingBinaries=true
if ($LASTEXITCODE -ne 0 ) {
    exit 1
}

# And run tests for a few platforms
if ($noWindows -ne $true) {
    & dotnet run -c Release --project "$PSScriptRoot\..\Examples\StereoKitTest\StereoKitTest.csproj" -- -test -screenfolder "$PSScriptRoot/Screenshots/"
    if ($LASTEXITCODE -ne 0 ) {
        exit 1
    }
}

if ($noLinux -ne $true) {
    $linux_folder = ''+$PSScriptRoot
    $linux_folder = $linux_folder.replace('\', '/')
    $linux_folder = $linux_folder.replace(':', '')
    $linux_folder = '/mnt/'+$linux_folder
    $linux_folder = $linux_folder.replace('/C/', '/c/')

    & wsl LIBGL_ALWAYS_SOFTWARE=1 dotnet run -c Release --project "$linux_folder/../Examples/StereoKitTest/StereoKitTest.csproj" -- -test -noscreens
    if ($LASTEXITCODE -ne 0 ) {
        exit 1
    }
}