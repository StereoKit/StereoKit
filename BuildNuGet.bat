SET vs="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe"

%vs% StereoKit.sln /Clean
%vs% StereoKit.sln /Build "Release|x64" /Project StereoKitC
%vs% StereoKit.sln /Build "Release|x64" /Project StereoKitC_UWP
%vs% StereoKit.sln /Build "Release|ARM64" /Project StereoKitC_UWP
%vs% StereoKit.sln /Build "Release|X64" /Project StereoKit
%vs% StereoKit.sln /Build "Release|ARM64" /Project StereoKit