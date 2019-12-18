Param([string]$folder)

# Replace names in the NetCore project
$from = 'SKTemplate_NetCore_Name'
$to   = '$safeprojectname$'

$project = 'SKTemplate_NetCore\'
$out     = $folder+'SKTemplate_NetCore\templatized\'
New-Item -Path ($out) -ItemType "directory" -ErrorAction SilentlyContinue
Get-ChildItem ($folder+$project+"*.*") | 
Foreach-Object {
    ((Get-Content -path $_.FullName -Raw) -replace $from,$to) | Set-Content -Path ($out+$_.Name)
}

# Replace names in the UWP project
$from1 = 'SKTemplate_UWP_Name'
$to1   = '$safeprojectname$'
$from2 = 'EB50E752-CF5E-4E7D-B81D-26A67CFCC6F2'
$to2   = '$guid1$'

#...Root project folder
$project = 'SKTemplate_UWP\'
$out     = $folder+'SKTemplate_UWP\templatized\'
New-Item -Path ($out) -ItemType "directory" -ErrorAction SilentlyContinue
Get-ChildItem ($folder+$project+"*.*") | 
Foreach-Object {
    ((Get-Content -path $_.FullName -Raw) -replace $from1,$to1 -replace $from2,$to2) | Set-Content -Path ($out+$_.Name)
}

#...Properties folder
$project = 'SKTemplate_UWP\Properties\'
$out     = $folder+'SKTemplate_UWP\templatized\Properties\'
New-Item -Path ($out) -ItemType "directory" -ErrorAction SilentlyContinue
Get-ChildItem ($folder+$project+"*.*") | 
Foreach-Object {
    ((Get-Content -path $_.FullName -Raw) -replace $from1,$to1 -replace $from2,$to2) | Set-Content -Path ($out+$_.Name)
}

#...Assets shouldn't have any strings that need replaced
$project = 'SKTemplate_UWP\Assets\'
$out     = $folder+'SKTemplate_UWP\templatized\Assets\'
New-Item -Path ($out) -ItemType "directory" -ErrorAction SilentlyContinue
Copy-Item ($folder+$project) -Destination $out -Recurse -Force

# Now Zip everything, and put them in the templates folder!
Compress-Archive -Path ($folder+"SKTemplate_NetCore\templatized\*.*") -DestinationPath ($folder+"SKTemplates\ProjectTemplates\StereoKit .Net Core.zip") -Force
Compress-Archive -Path ($folder+"SKTemplate_UWP\templatized\*.*"),($folder+"SKTemplate_UWP\templatized\Assets\"),($folder+"SKTemplate_UWP\templatized\Properties\") -DestinationPath ($folder+"SKTemplates\ProjectTemplates\StereoKit UWP.zip") -Force