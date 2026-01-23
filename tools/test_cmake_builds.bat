cls

cd ..
cmake --preset Win32_x64_Release
cmake --build --preset Win32_x64_Release

cmake --preset Android_Arm64_Release
cmake --build --preset Android_Arm64_Release

wsl cmake --preset Linux_x64_Release
wsl cmake --build --preset Linux_x64_Release
cd tools