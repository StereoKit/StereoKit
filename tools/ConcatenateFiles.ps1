# From .\StereoKit, invoke this as:
# .\tools\ConcatenateFiles.ps1 -SourceFolder ".\StereoKitC" -OutputFile ".\tools\stereokit.merged.txt" -ExcludeDirs @("StereoKitC_UWP", "lib\include") -ExcludeFiles @("libraries\miniaudio.h", "libraries\miniaudio.cpp", "libraries\cgltf.h", "libraries\cgltf.cpp", "libraries\qoi.h", "libraries\qoi.cpp", "libraries\stb.cpp", "libraries\stb_truetype.h", "libraries\stb_image.h", "libraries\stb_image_write.h", "libraries\sokol_time.h", "libraries\sokol_time.cpp")
<#
.SYNOPSIS
    Combines all .h and .cpp files in a specified folder (recursively) into a single file with separators.

.DESCRIPTION
    This script searches through a given directory and its subdirectories for all `.h` and `.cpp` files,
    excluding any directories and specific files specified by the user. It then combines these files into
    one output file, inserting a clear separator between each file's content for easy identification.

.PARAMETER SourceFolder
    The path to the folder containing the `.h` and `.cpp` files. Defaults to the current directory.

.PARAMETER OutputFile
    The path and name of the output file. Defaults to `CombinedFiles.cpp` in the current directory.

.PARAMETER ExcludeDirs
    An array of directory names or paths to exclude from the search. Provide relative or absolute paths.

.PARAMETER ExcludeFiles
    An array of file names or paths to exclude from the combination. Provide relative paths (relative to SourceFolder) or absolute paths.

.EXAMPLE
    .\CombineCppHFiles.ps1 -SourceFolder "C:\Projects\MyApp" -OutputFile "C:\Projects\Combined.cpp" -ExcludeDirs @("Tests", "ThirdParty") -ExcludeFiles @("main.cpp", "utils.h")

.NOTES
    - Ensure you have the necessary permissions to read the source files and write to the output location.
    - This script overwrites the output file if it already exists.
#>

param(
    [Parameter(Mandatory = $false)]
    [string]$SourceFolder = ".",

    [Parameter(Mandatory = $false)]
    [string]$OutputFile = "CombinedFiles.cpp",

    [Parameter(Mandatory = $false)]
    [string[]]$ExcludeDirs = @(),

    [Parameter(Mandatory = $false)]
    [string[]]$ExcludeFiles = @()
)

# Function to compute relative paths using [System.Uri] for compatibility
function Get-RelativePathUsingUri {
    param (
        [string]$FromPath,
        [string]$ToPath
    )

    # Ensure both paths are absolute
    $fromUri = New-Object System.Uri ($FromPath.EndsWith('\') -or $FromPath.EndsWith('/') ? $FromPath : "$FromPath\")
    $toUri = New-Object System.Uri $ToPath
    $relativeUri = $fromUri.MakeRelativeUri($toUri)
    $relativePath = [System.Uri]::UnescapeDataString($relativeUri.ToString()).Replace('/', '\')
    return $relativePath
}

# Check PowerShell version for GetRelativePath availability
$hasGetRelativePath = $true;# [System.IO.Path]::GetMethod("GetRelativePath", [Type[]]@([string], [string])) -ne $null

# Resolve the absolute path of the source folder
try {
    $resolvedSource = Resolve-Path $SourceFolder -ErrorAction Stop
    $sourcePath = $resolvedSource.Path
    if (-not $sourcePath) {
        Write-Error "Resolved SourceFolder path is empty."
        exit 1
    }
    Write-Verbose "SourceFolder resolved to: $sourcePath"
}
catch {
    Write-Error "Failed to resolve SourceFolder '$SourceFolder'. $_"
    exit 1
}

# Initialize the output file by removing it if it already exists
if (Test-Path $OutputFile) {
    try {
        Remove-Item -Path $OutputFile -Force
        Write-Host "Existing output file '$OutputFile' removed."
    }
    catch {
        Write-Error "Failed to remove existing output file '$OutputFile'. $_"
        exit 1
    }
}

# Convert ExcludeDirs to absolute paths for accurate comparison
$ExcludeDirPaths = @()
foreach ($dir in $ExcludeDirs) {
    try {
        if ([System.IO.Path]::IsPathRooted($dir)) {
            if (Test-Path $dir -PathType Container) {
                $resolvedDir = Resolve-Path $dir -ErrorAction Stop
                $ExcludeDirPaths += $resolvedDir.Path.TrimEnd('\')
                Write-Verbose "Excluded directory added: $($resolvedDir.Path)"
            }
            else {
                Write-Warning "Excluded directory '$dir' does not exist or is not a directory."
            }
        }
        else {
            $resolvedDir = Join-Path $sourcePath $dir
            if (Test-Path $resolvedDir -PathType Container) {
                $resolvedDir = Resolve-Path $resolvedDir -ErrorAction Stop
                $ExcludeDirPaths += $resolvedDir.Path.TrimEnd('\')
                Write-Verbose "Excluded directory added: $($resolvedDir.Path)"
            }
            else {
                Write-Warning "Excluded directory '$dir' does not exist under SourceFolder."
            }
        }
    }
    catch {
        Write-Warning "Failed to resolve excluded directory '$dir'. $_"
    }
}

# Process ExcludeFiles to handle both full paths and filenames
$ExcludeFilePaths = @()
$ExcludeFileNames = @()
foreach ($file in $ExcludeFiles) {
    try {
        if ([System.IO.Path]::IsPathRooted($file)) {
            if (Test-Path $file -PathType Leaf) {
                $resolvedFile = Resolve-Path $file -ErrorAction Stop
                $ExcludeFilePaths += $resolvedFile.Path
                Write-Verbose "Excluded file added by path: $($resolvedFile.Path)"
            }
            else {
                Write-Warning "Excluded file '$file' does not exist."
            }
        }
        else {
            $resolvedFile = Join-Path $sourcePath $file
            if (Test-Path $resolvedFile -PathType Leaf) {
                $resolvedFile = Resolve-Path $resolvedFile -ErrorAction Stop
                $ExcludeFilePaths += $resolvedFile.Path
                Write-Verbose "Excluded file added by path: $($resolvedFile.Path)"
            }
            else {
                # If the file does not exist at the relative path, treat it as a filename to exclude
                $ExcludeFileNames += $file
                Write-Warning "Excluded file '$file' does not exist under SourceFolder. It will be excluded by filename."
            }
        }
    }
    catch {
        Write-Warning "Failed to resolve excluded file '$file'. $_"
    }
}

# Function to determine if a file is within any excluded directory
function IsExcludedDir($filePath, $excludedDirs) {
    foreach ($excluded in $excludedDirs) {
        # Ensure directory paths end with a backslash for accurate matching
        $excludedDir = if ($excluded.EndsWith('\') -or $excluded.EndsWith('/')) { $excluded } else { "$excluded\" }
        if ($filePath.StartsWith($excludedDir, [System.StringComparison]::InvariantCultureIgnoreCase)) {
            return $true
        }
    }
    return $false
}

# Function to determine if a file is in the excluded files list
function IsExcludedFile($filePath, $excludedFilePaths, $excludedFileNames) {
    # Check if the full path is excluded
    foreach ($excluded in $excludedFilePaths) {
        if ($filePath -ieq $excluded) {
            return $true
        }
    }
    # Check if the filename is excluded
    $fileName = [System.IO.Path]::GetFileName($filePath)
    foreach ($excludedName in $excludedFileNames) {
        if ($fileName -ieq $excludedName) {
            return $true
        }
    }
    return $false
}

# Retrieve all .h and .cpp files recursively, excluding specified directories and files
$files = Get-ChildItem -Path $sourcePath -Recurse -Include *.h, *.cpp -File | Where-Object {
    -not (IsExcludedDir $_.FullName $ExcludeDirPaths) -and
    -not (IsExcludedFile $_.FullName $ExcludeFilePaths $ExcludeFileNames)
}

if ($files.Count -eq 0) {
    Write-Host "No .h or .cpp files found in '$sourcePath' excluding specified directories and files."
    exit 0
}

Write-Host "Combining $($files.Count) files into '$OutputFile'..."

foreach ($file in $files | Sort-Object FullName) {
    # Create a relative path for the separator
    if ($hasGetRelativePath) {
        try {
            $relativePath = [System.IO.Path]::GetRelativePath($sourcePath, $file.FullName)
        }
        catch {
            Write-Warning "Failed to get relative path using GetRelativePath. Falling back to URI method. $_"
            $relativePath = Get-RelativePathUsingUri -FromPath $sourcePath -ToPath $file.FullName
        }
    }
    else {
        $relativePath = Get-RelativePathUsingUri -FromPath $sourcePath -ToPath $file.FullName
    }

    # Write separator
    $separator = @"
////////////////////////////////////////
// Begin File: $relativePath
////////////////////////////////////////

"@
    Add-Content -Path $OutputFile -Value $separator

    # Append file content
    try {
        Get-Content -Path $file.FullName | Add-Content -Path $OutputFile
        Add-Content -Path $OutputFile -Value "`n"  # Add a newline for readability
        Write-Host "Added '$relativePath'"
    }
    catch {
        Write-Warning "Failed to add '$relativePath'. $_"
    }
}

Write-Host "All files have been successfully combined into '$OutputFile'."
