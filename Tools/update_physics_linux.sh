#!/bin/bash
git clone https://github.com/DanielChappuis/reactphysics3d.git
cd reactphysics3d

# Replace the include files
rm -r -f "../../StereoKitC/lib/include/reactphysics3d"
mkdir "../../StereoKitC/lib/include/reactphysics3d"
cp -r "include/reactphysics3d" "../../StereoKitC/lib/include/reactphysics3d"

# Build linux versions
mkdir Release_x64_linux
cd Release_x64_linux
cmake \
 -DCMAKE_BUILD_TYPE=Release \
 ../
cmake --build . --config Release
cp libreactphysics3d.a "../../../StereoKitC/lib/bin/x64/Release/libreactphysics3d.a"
cd ..

mkdir Debug_x64_linux
cd Debug_x64_linux
cmake \
 -DCMAKE_BUILD_TYPE=Debug \
 ../
cmake --build .
cp libreactphysics3d.a "../../../StereoKitC/lib/bin/x64/Debug/libreactphysics3d.a"
cd ..

cd ..

# And delete it all when we're done
rm -r -f reactphysics3d