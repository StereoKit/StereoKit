git clone https://github.com/DanielChappuis/reactphysics3d.git
cd reactphysics3d/

:: Replace the include files
rmdir /S /Q "../../StereoKitC/lib/include/reactphysics3d"
mkdir "../../StereoKitC/lib/include/reactphysics3d"
xcopy "include/reactphysics3d" "../../StereoKitC/lib/include/reactphysics3d" /E

:: Build x64 Release/Debug, ARM64 Release/Debug and ARM Release/Debug with Visual Studio
mkdir Release_x64
cd Release_x64
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
xcopy /Y /F "Release/reactphysics3d.lib" "../../../StereoKitC/lib/bin/x64/Release/reactphysics3d.lib"
cd ..

mkdir Debug_x64
cd Debug_x64
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
xcopy /Y /F "Debug/reactphysics3d.lib" "../../../StereoKitC/lib/bin/x64/Debug/reactphysics3d.lib"
xcopy /Y /F "Debug/reactphysics3d.pdb" "../../../StereoKitC/lib/bin/x64/Debug/reactphysics3d.pdb"
cd ..

mkdir Release_ARM64
cd Release_ARM64
cmake -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
xcopy /Y /F "Release/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM64/Release/reactphysics3d.lib"
cd ..

mkdir Debug_ARM64
cd Debug_ARM64
cmake -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
xcopy /Y /F "Debug/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM64/Debug/reactphysics3d.lib"
xcopy /Y /F "Debug/reactphysics3d.pdb" "../../../StereoKitC/lib/bin/ARM64/Debug/reactphysics3d.pdb"
cd ..

mkdir Release_ARM
cd Release_ARM
cmake -G "Visual Studio 16 2019" -A ARM -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
xcopy /Y /F "Release/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM/Release/reactphysics3d.lib"
cd ..

mkdir Debug_ARM
cd Debug_ARM
cmake -G "Visual Studio 16 2019" -A ARM -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
xcopy /Y /F "Debug/reactphysics3d.lib" "../../../StereoKitC/lib/bin/ARM/Debug/reactphysics3d.lib"
xcopy /Y /F "Debug/reactphysics3d.pdb" "../../../StereoKitC/lib/bin/ARM/Debug/reactphysics3d.pdb"
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
 xcopy /Y /F libreactphysics3d.a "../../../StereoKitC/lib/bin/arm64-v8a/Release/libreactphysics3d.a"
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
 xcopy /Y /F libreactphysics3d.a "../../../StereoKitC/lib/bin/arm64-v8a/Debug/libreactphysics3d.a"
cd ..

cd ..

:: And delete it all when we're done
rmdir /S /Q reactphysics3d