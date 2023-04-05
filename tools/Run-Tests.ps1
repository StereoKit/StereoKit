$linux_folder = ''+$PSScriptRoot
$linux_folder = $linux_folder.replace('\', '/')
$linux_folder = $linux_folder.replace(':', '')
$linux_folder = '/mnt/'+$linux_folder
$linux_folder = $linux_folder.replace('/C/', '/c/')

cmd /c "wsl cd '${linux_folder}/../' ; xmake f -p linux -a x64 -m release -y ; xmake ;"
cmd /c "wsl cd '${linux_folder}/../Examples/StereoKitTest/' ; dotnet restore StereoKitTest.csproj ; dotnet clean -p:SKTest=True StereoKitTest.csproj ; dotnet build -p:SKTest=True StereoKitTest.csproj ; cd bin ; LIBGL_ALWAYS_SOFTWARE=1 dotnet run -p:SKTest=True --project ../StereoKitTest.csproj -test -screenfolder '${linux_folder}/Screenshots/Linux'"

& 'cd' '../Examples/StereoKitTest/'
& 'dotnet' 'restore' 'StereoKitTest.csproj'
& 'dotnet' 'clean' '-p:SKTest=True' 'StereoKitTest.csproj'
& 'dotnet' 'build' '-p:SKTest=True' 'StereoKitTest.csproj'
& 'dotnet' 'run' '-p:SKTest=True' '--project' 'StereoKitTest.csproj' '-test' '-screenfolder' "${PSScriptRoot}\Screenshots\Windows"
& 'cd' '../../Tools/'