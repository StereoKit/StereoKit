
function File-Length($path) {
    $result = 0
    if (Test-Path -Path $path) {
        $result = (Get-Item -Path $path).length
    }
    return $result
}

function Unix-Size-Summary($report) {
    if ($report -eq '') { return '' }

    $sizes = @{}
    ($report | Select-String -Pattern "\w* ([0-9a-fA-F]*) \w* ([^_:<(]*)(?:[_:<(]*)([^_:<(]*)" -AllMatches).Matches | ForEach-Object {
        $sizes[$_.Groups[2].Value] += [System.Convert]::ToInt64($_.Groups[1].Value, 16)
    }

    $breakdown = "|      Prefix/Function | kb   |
| -------------------- | ---- |
"
    $total         = 0
    $summary_total = 0
    $sizes.GetEnumerator() | Sort-Object Value -Descending | ForEach-Object {
        $total += $_.Value
        if ($_.Value -gt 10kb) { 
            $summary_total += $_.Value
            $size = [math]::Round($_.Value/1kb)
            $breakdown += "| {0,20:N0} | {1,-4:N0} |`n" -f $_.Key, $size
        }
    }
    $total         = [math]::Round($total/1kb)
    $summary_total = [math]::Round($summary_total/1kb)
    $breakdown     = "This summary represents $summary_total/$total kb, or {0:N1}%.`n`n$breakdown" -f (($summary_total/$total)*100)
    return $breakdown
}

# Get size of all the binaries
$rootFolder = "$PSScriptRoot/../bin/distribute/bin"
$size_x64        = File-Length "$rootFolder/Win32/x64/Release/StereoKitC.dll"
$size_arm64      = File-Length "$rootFolder/Win32/ARM64/Release/StereoKitC.dll"
$size_x64_and    = File-Length "$rootFolder/Android/x86_64/Release/libStereoKitC.so"
$size_x64_linux  = File-Length "$rootFolder/Linux/x64/Release/libStereoKitC.so"
$size_arm64_and  = File-Length "$rootFolder/Android/arm64-v8a/Release/libStereoKitC.so"
$size_arm64_linux= File-Length "$rootFolder/Linux/arm64/Release/libStereoKitC.so"

$build_sizes = (@"
## Build Sizes:

| Platform | Arch  | Size, kb | Size, bytes |
| -------- | ----- | -------- | ----------- |
| Win32    | x64   | {0,8:N0} | {1,11:N0} |
| Win32    | ARM64 | {2,8:N0} | {3,11:N0} |
| Linux    | x64   | {10,8:N0} | {11,11:N0} |
| Linux    | ARM64 | {12,8:N0} | {13,11:N0} |
| Android  | x64   | {14,8:N0} | {15,11:N0} |
| Android  | ARM64 | {16,8:N0} | {17,11:N0} |
"@ -f ([math]::Round($size_x64        /1kb), $size_x64,
       [math]::Round($size_arm64      /1kb), $size_arm64,
       [math]::Round($size_x64_linux  /1kb), $size_x64_linux,
       [math]::Round($size_arm64_linux/1kb), $size_arm64_linux,
       [math]::Round($size_x64_and    /1kb), $size_x64_and,
       [math]::Round($size_arm64_and  /1kb), $size_arm64_and
       ))

$linux_x64_report = ''
$android_arm64_report = ''
# Get a binary breakdown of Android and Linux
if (Get-Command "nm" -ErrorAction SilentlyContinue) {
    $linux_x64_report      = & nm -CSr --size-sort $PSScriptRoot/../bin/distribute/bin/Linux/x64/Release/libStereoKitC.so.dbg
    $android_arm64_report  = & nm -CSr --size-sort $PSScriptRoot/../bin/distribute/bin/Android/arm64-v8a/Release/libStereoKitC.so.dbg
} elseif (Get-Command "wsl" -ErrorAction SilentlyContinue) {
    $linux_folder = ''+$PSScriptRoot
    $linux_folder = $linux_folder.replace('\', '/')
    $linux_folder = $linux_folder.replace(':', '')
    $linux_folder = '/mnt/'+$linux_folder
    $linux_folder = $linux_folder.replace('/C/', '/c/')
    $linux_x64_report      = & wsl nm -CSr --size-sort $linux_folder/../bin/distribute/bin/Linux/x64/Release/libStereoKitC.so.dbg
    $android_arm64_report  = & wsl nm -CSr --size-sort $linux_folder/../bin/distribute/bin/Android/arm64-v8a/Release/libStereoKitC.so.dbg
}
$linux_x64_summary     = Unix-Size-Summary $linux_x64_report 
$android_arm64_summary = Unix-Size-Summary $android_arm64_report

# Write up a markdown summary
$build_info = "# StereoKit Build Information

$build_sizes
"
if ($linux_x64_summary -ne '') {
    $build_info += "## Linux x64 Binary Breakdown

$linux_x64_summary"
}
if ($android_arm64_summary -ne '') {
    $build_info += "## Android arm64 Binary Breakdown

$android_arm64_summary"
}

# Write it to file, and print it
Set-Content -path "$PSScriptRoot/BuildInfo.md" -value $build_info
Write-Host $build_info