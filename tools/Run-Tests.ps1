<#
.SYNOPSIS
Runs StereoKit's C# tests on every platform this machine can reach.

.DESCRIPTION
Each host tests its own platform natively, and the other platform via a
compatibility layer: WSL runs the Linux build on Windows, and Wine runs the
Windows build on Linux. The native leg also runs in XR mode when monado-service
is on PATH; cross legs are Simulator-only.

Legs that need something this machine doesn't have are skipped, not failed.

.PARAMETER noNative
Skip the host-native legs.

.PARAMETER noCross
Skip the WSL (on Windows) or Wine (on Linux) leg.

.PARAMETER noXR
Skip the monado XR leg, running Simulator only.

.PARAMETER configuration
Build configuration to test, Release or Debug.
#>
param(
    [switch]$noNative = $false,
    [switch]$noCross  = $false,
    [switch]$noXR     = $false,
    [ValidateSet('Release', 'Debug')]
    [string]$configuration = 'Release'
)

$repoRoot   = (Resolve-Path "$PSScriptRoot/..").Path
$testProj   = Join-Path $repoRoot 'Examples/StereoKitTest/StereoKitTest.csproj'
$skProj     = Join-Path $repoRoot 'StereoKit/StereoKit.csproj'
$testArgs   = @('-test', '-noscreens')
# $IsWindows only exists in PowerShell Core; Windows PowerShell 5.1 is Windows.
$hostIsWin  = $IsWindows -or ($null -eq $IsWindows)
$skipped    = @()

function Write-Leg  ($name) { Write-Host "`n=== $name ===" -ForegroundColor Cyan }
function Write-Pass ($name) { Write-Host "PASSED: $name"   -ForegroundColor Green }

function Write-Skip {
    param($name, $why)
    Write-Host "SKIPPED: $name - $why" -ForegroundColor Yellow
    $script:skipped += "$name ($why)"
}

function Assert-Ok {
    param($name)
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAILED: $name (exit $LASTEXITCODE)" -ForegroundColor Red
        exit 1
    }
}

function Test-Tool ($name) { $null -ne (Get-Command $name -ErrorAction SilentlyContinue) }

# An installed monado usually isn't the active OpenXR runtime.
function Get-MonadoRuntimeJson {
    $service = Get-Command 'monado-service' -ErrorAction SilentlyContinue
    if ($null -eq $service) { return $null }

    $installRoot = Split-Path (Split-Path $service.Source -Parent) -Parent
    $candidates  = @(
        (Join-Path $installRoot 'share/openxr/1/openxr_monado.json'),
        '/usr/local/share/openxr/1/openxr_monado.json',
        '/usr/share/openxr/1/openxr_monado.json')
    foreach ($path in $candidates) {
        if (Test-Path $path) { return (Resolve-Path $path).Path }
    }
    return $null
}

# monado-service epolls stdin, which only works on a pipe, so Start-Process's
# file redirection can't be used. Closing that pipe is also how it's stopped.
function Start-Monado {
    $info = [System.Diagnostics.ProcessStartInfo]::new()
    $info.FileName               = 'monado-service'
    $info.UseShellExecute        = $false
    $info.RedirectStandardInput  = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError  = $true

    $proc = [System.Diagnostics.Process]::Start($info)
    # Drain both pipes, or a chatty service blocks once a buffer fills.
    $out  = $proc.StandardOutput.ReadToEndAsync()
    $err  = $proc.StandardError.ReadToEndAsync()

    $socket = Join-Path $env:XDG_RUNTIME_DIR 'monado_comp_ipc'
    for ($i = 0; $i -lt 100; $i++) {
        if (Test-Path $socket) { return @{ proc = $proc; out = $out; err = $err } }
        if ($proc.HasExited)   { break }
        Start-Sleep -Milliseconds 100
    }

    Write-Host "monado-service did not come up:" -ForegroundColor Red
    Write-Host $out.GetAwaiter().GetResult()
    Write-Host $err.GetAwaiter().GetResult()
    if (-not $proc.HasExited) { $proc.Kill() }
    return $null
}

function Stop-Monado {
    param($monado)
    $proc = $monado.proc
    if ($proc.HasExited) { return }
    $proc.StandardInput.Close()
    if (-not $proc.WaitForExit(5000)) { $proc.Kill() }
}

# Turn C:\a\b into /mnt/c/a/b so WSL can reach the repo.
function ConvertTo-WslPath {
    param([string]$path)
    $wsl = $path.Replace('\', '/').Replace(':', '')
    return '/mnt/' + $wsl.Substring(0, 1).ToLower() + $wsl.Substring(1)
}

Write-Host "`nRunning Tests! ($configuration)"

# Build a non-nuget version of C#'s StereoKit.dll for testing.
& dotnet build $skProj -c $configuration -p:SKIgnoreMissingBinaries=true
Assert-Ok 'StereoKit.dll build'

########################################
## Native, in Simulator and XR        ##
########################################

if ($noNative -ne $true) {
    $name = if ($hostIsWin) { 'Windows native, Simulator' } else { 'Linux native, Simulator' }
    Write-Leg $name
    & dotnet run -c $configuration --project $testProj -- @testArgs
    Assert-Ok $name
    Write-Pass $name

    $name = if ($hostIsWin) { 'Windows native, XR (monado)' } else { 'Linux native, XR (monado)' }
    $runtimeJson = Get-MonadoRuntimeJson
    if ($noXR -eq $true) {
        Write-Skip $name '-noXR was passed'
    } elseif ($null -eq $runtimeJson) {
        Write-Skip $name 'monado-service or its openxr_monado.json was not found'
    } else {
        Write-Leg $name
        $monado = Start-Monado
        if ($null -eq $monado) {
            Write-Host "FAILED: $name" -ForegroundColor Red
            exit 1
        }
        try {
            $env:XR_RUNTIME_JSON = $runtimeJson
            & dotnet run -c $configuration --project $testProj -- @testArgs -xr
            Assert-Ok $name
        } finally {
            Remove-Item Env:\XR_RUNTIME_JSON -ErrorAction SilentlyContinue
            Stop-Monado $monado
        }
        Write-Pass $name
    }
}

########################################
## Cross-platform, Simulator only     ##
########################################

if ($noCross -eq $true) {
    Write-Skip 'Cross-platform leg' '-noCross was passed'
} elseif ($hostIsWin) {
    $name = 'Linux via WSL, Simulator'
    if (-not (Test-Tool 'wsl')) {
        Write-Skip $name 'wsl is not on PATH'
    } else {
        Write-Leg $name
        $wslProj = ConvertTo-WslPath $testProj
        & wsl LIBGL_ALWAYS_SOFTWARE=1 dotnet run -c $configuration --project $wslProj -- @testArgs
        Assert-Ok $name
        Write-Pass $name
    }
} else {
    $name = 'Windows via Wine, Simulator'
    if (-not (Test-Tool 'wine')) {
        Write-Skip $name 'wine is not on PATH'
    } elseif (-not (Test-Tool 'x86_64-w64-mingw32-g++')) {
        Write-Skip $name 'the mingw-w64 toolchain is not on PATH'
    } else {
        Write-Leg $name

        # Tests on: we never run these here, but building StereoKitCTest and
        # SKTests keeps the C API honest and link-checks the C++ side.
        $preset = "MinGW_x64_$configuration"
        & cmake --preset $preset -DSK_BUILD_TESTS=ON
        Assert-Ok 'MinGW configure'
        & cmake --build --preset $preset
        Assert-Ok 'MinGW build'

        # SKSDKBuildOS would otherwise follow the host and pack the .so, and
        # SKSDKPreset would derive the MSVC preset name, which needs Windows.
        $publishDir = Join-Path $repoRoot "bin/x64_$configuration/StereoKitTest_win"
        & dotnet publish $testProj -c $configuration -r win-x64 --self-contained `
            -p:SKSDKBuildOS=Win32 -p:SKSDKPreset=$preset -o $publishDir
        Assert-Ok 'win-x64 publish'

        Push-Location $publishDir
        try {
            & wine './StereoKitTest.exe' @testArgs
            Assert-Ok $name
        } finally {
            Pop-Location
        }
        Write-Pass $name
    }
}

Write-Host "`nAll tests passed!" -ForegroundColor Green
if ($skipped.Count -gt 0) {
    Write-Host "Skipped $($skipped.Count) leg(s):" -ForegroundColor Yellow
    foreach ($skip in $skipped) { Write-Host "  - $skip" -ForegroundColor Yellow }
}
