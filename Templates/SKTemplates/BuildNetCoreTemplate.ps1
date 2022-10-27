Param([string]$folder)

function Build {
    param($srcFolder, $dstFolder, $dstZip, $replaceArray)

    # Ensure the destination folder exists and is empty
    New-Item -Path $dstFolder -ItemType "directory" -ErrorAction SilentlyContinue
    Remove-Item -Recurse -Force $dstFolder

    Get-ChildItem "$srcFolder*.*" -Recurse | 
    Foreach-Object {
        # Skip any pre-built content
        if ($_.FullName.Contains('\bin\') -or $_.FullName.Contains('\obj\') -or $_.FullName.EndsWith('.user')) {
            return
        }

        # Get the destination name and ensure its folder exists
        $relativeName = $_.FullName.Substring($_.FullName.IndexOf($srcFolder) + $srcFolder.Length)
        $directory    = [System.IO.Path]::GetDirectoryName($relativeName)
        New-Item -Path ($dstFolder+$directory) -ItemType "directory" -ErrorAction SilentlyContinue

        # Load, string replace, and write file
        $fileContent = Get-Content -path $_.FullName -Raw
        for ($i = 0; $i -lt $replaceArray.Count; $i+=2) {
            $fileContent = $fileContent.Replace($replaceArray[$i], $replaceArray[$i+1])
        }
        Set-Content -Path ($dstFolder+$relativeName) -Value $fileContent
    }

    # Zip the result and delete the folder!
    Compress-Archive -Path ($dstFolder+'\*') -DestinationPath $dstZip -Force
    Remove-Item -Recurse -Force $dstFolder
}

###########################################

Push-Location -Path '..'

Build `
    -srcFolder ($folder+'SKTemplate_NetCore\') `
    -dstFolder ($folder+'SKTemplate_NetCore_templatized\') `
    -dstZip    ($folder+"SKTemplates\ProjectTemplates\StereoKit .Net Core.zip") `
    -replaceArray [System.Collections.ArrayList]@( `
        'SKTemplate_NetCore_Name', '$safeprojectname$')

Build `
    -srcFolder ($folder+'SKTemplate_UWP\') `
    -dstFolder ($folder+'SKTemplate_UWP_templatized\') `
    -dstZip    ($folder+"SKTemplates\ProjectTemplates\StereoKit UWP.zip") `
    -replaceArray [System.Collections.ArrayList]@( `
        'SKTemplate_UWP_Name',                  '$safeprojectname$',`
        'EB50E752-CF5E-4E7D-B81D-26A67CFCC6F2', '$guid1$',`
        'a1d049e3-609d-456e-a2a3-ea2e06f20b1d', '$guid2$',`
        'CN=niklinge',                          'CN=$username$')

Build `
    -srcFolder ($folder+'SKTemplate_Maui\') `
    -dstFolder ($folder+'SKTemplate_Maui_templatized\') `
    -dstZip    ($folder+"SKTemplates\ProjectTemplates\StereoKit Multitarget.zip") `
    -replaceArray [System.Collections.ArrayList]@( `
        'SKTemplate_Maui',                      '$safeprojectname$',`
        '2C92FB1D-FDD8-4151-9291-8F1118FCF501', '$guid1$',`
        'User_Name',                            '$username$')

Pop-Location