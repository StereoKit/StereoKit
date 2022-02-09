git clone https://github.com/DanielChappuis/reactphysics3d.git
cd reactphysics3d/
:: v0.9.0
git checkout 4bbbaa7c6e92942734eec696e23a2fad1f1cb8a1

:: Replace the include files
rmdir /S /Q "../../StereoKitC/lib/include/reactphysics3d"
mkdir "../../StereoKitC/lib/include/reactphysics3d"
xcopy "include/reactphysics3d" "../../StereoKitC/lib/include/reactphysics3d" /E

:: Build x64 Release/Debug, ARM64 Release/Debug and ARM Release/Debug with Visual Studio
mkdir Release_x64
cd Release_x64
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
xcopy /Y /F "Release/reactphysics3d.lib" "../../../StereoKitC/lib/bin/x64/Release/*"
cd ..

mkdir Debug_x64
cd Debug_x64
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
xcopy /Y /F "Debug/reactphysics3d.lib" "../../../StereoKitC/lib/bin/x64/Debug/*"
xcopy /Y /F "Debug/reactphysics3d.pdb" "../../../StereoKitC/lib/bin/x64/Debug/*"
cd ..

mkdir Release_ARM64
cd Release_ARM64
cmake -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
xcopy /Y /F "Release/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM64/Release/*"
cd ..

mkdir Debug_ARM64
cd Debug_ARM64
cmake -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
xcopy /Y /F "Debug/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM64/Debug/*"
xcopy /Y /F "Debug/reactphysics3d.pdb" "../../../StereoKitC/lib/bin/ARM64/Debug/*"
cd ..

mkdir Release_ARM
cd Release_ARM
cmake -G "Visual Studio 16 2019" -A ARM -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
xcopy /Y /F "Release/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM/Release/*"
cd ..

mkdir Debug_ARM
cd Debug_ARM
cmake -G "Visual Studio 16 2019" -A ARM -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
xcopy /Y /F "Debug/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM/Debug/*"
xcopy /Y /F "Debug/reactphysics3d.pdb" "../../../StereoKitC/lib/bin/ARM/Debug/*"
cd ..

:: Build Android versions using the NDK
:: See this doc for reference: https://developer.android.com/ndk/guides/cmake
mkdir Release_arm64-v8a
cd Release_arm64-v8a
"C:/Users/progr/AppData/Local/Android/Sdk/cmake/3.10.2.4988404/bin/cmake.exe"^
 -G"Ninja"^
 -DCMAKE_MAKE_PROGRAM="C:/Users/progr/AppData/Local/Android/Sdk/cmake/3.10.2.4988404/bin/ninja.exe"^
 -DCMAKE_TOOLCHAIN_FILE=C:/Microsoft/AndroidNDK64/android-ndk-r16b/build/cmake/android.toolchain.cmake^
 -DANDROID_ABI=arm64-v8a^
 -DANDROID_NATIVE_API_LEVEL=24^
 -DANDROID_TOOLCHAIN=clang^
 -DCMAKE_BUILD_TYPE=Release^
 -DANDROID_STL=c++_static^
 ../
 "C:/Users/progr/AppData/Local/Android/Sdk/cmake/3.10.2.4988404/bin/cmake.exe" --build . --config Release
 xcopy /Y /F libreactphysics3d.a "../../../StereoKitC/lib/bin/arm64-v8a/Release/*"
cd ..

mkdir Debug_arm64-v8a
cd Debug_arm64-v8a
"C:/Users/progr/AppData/Local/Android/Sdk/cmake/3.10.2.4988404/bin/cmake.exe"^
 -G"Ninja"^
 -DCMAKE_MAKE_PROGRAM="C:/Users/progr/AppData/Local/Android/Sdk/cmake/3.10.2.4988404/bin/ninja.exe"^
 -DCMAKE_TOOLCHAIN_FILE=C:/Microsoft/AndroidNDK64/android-ndk-r16b/build/cmake/android.toolchain.cmake^
 -DANDROID_ABI=arm64-v8a^
 -DANDROID_NATIVE_API_LEVEL=24^
 -DANDROID_TOOLCHAIN=clang^
 -DCMAKE_BUILD_TYPE=Debug^
 -DANDROID_STL=c++_static^
 ../
 "C:/Users/progr/AppData/Local/Android/Sdk/cmake/3.10.2.4988404/bin/cmake.exe" --build .
 xcopy /Y /F libreactphysics3d.a "../../../StereoKitC/lib/bin/arm64-v8a/Debug/*"
cd ..

cd ..

:: And delete it all when we're done
rmdir /S /Q reactphysics3d