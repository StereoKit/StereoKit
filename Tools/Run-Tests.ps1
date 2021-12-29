$linux_folder = ''+$PSScriptRoot
$linux_folder = $linux_folder.replace('\', '/')
$linux_folder = $linux_folder.replace(':', '')
$linux_folder = '/mnt/'+$linux_folder
$linux_folder = $linux_folder.replace('/C/', '/c/')
Write-Output $linux_folder

cmd /c "wsl cd '${linux_folder}/../' ; xmake f -p linux -a x64 -m release -y ; xmake ;"
cmd /c "wsl cd '${linux_folder}/../Examples/StereoKitTest/' ; dotnet build -p:OutputPath=bin StereoKitTest.csproj ; cd bin ; LIBGL_ALWAYS_SOFTWARE=1 dotnet run --project ../StereoKitTest.csproj -test -screenfolder ../../../Tools/Screenshots/Linux"

& 'cd' '../Examples/StereoKitTest/'
& 'dotnet' 'build' '-p:OutputPath=bin' 'StereoKitTest.csproj'
& 'cd' 'bin'
& 'dotnet' 'run' '-p:OutputPath=bin' '--project' '../StereoKitTest.csproj' '-test' '-screenfolder' '../../../Tools/Screenshots/Windows'
& 'cd' '../../../Tools/'