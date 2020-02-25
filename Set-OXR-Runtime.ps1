#Requires -RunAsAdministrator
param(
    [switch]$wmr = $false,
    [switch]$oculus = $false
)

$key     = "HKLM:\SOFTWARE\Khronos\OpenXR\1"
$name    = "ActiveRuntime"
$runtime = ""

if ($wmr -eq $true) {
    $runtime = "C:\WINDOWS\system32\MixedRealityRuntime.json"
} elseif ($oculus -eq $true) {
    $runtime = "C:\Program Files\Oculus\Support\oculus-runtime\oculus_openxr_64.json"
} else {
    Write-Host "Please specify a runtime switch! -WMR or -Oculus" -ForegroundColor red
    exit
}

Write-Host "Switching to $runtime"
Set-ItemProperty -Path $key -Name $name -Value $runtime