# To use this template, 
cmake_minimum_required(VERSION 3.10)
project(SKNativeTemplate VERSION "0.1.0" LANGUAGES CXX C)

# Get dependencies using CPM, this is grabbing StereoKit
# from the Github repository. You may want to consider 
# setting the CPM_SOURCE_CACHE environment variable as
# described by the CPM docs.
# See: https://github.com/cpm-cmake/CPM.cmake
include("cmake/CPM.cmake")
CPMAddPackage(
  NAME StereoKitC
  GITHUB_REPOSITORY maluoi/StereoKit
  GIT_TAG dd1f7f65f33df010c9838a25ffe0b5ad57ae3ae0 # /develop branch
  OPTIONS
    "SK_BUILD_TESTS OFF"
    "SK_BUILD_SHARED_LIBS OFF"
)

# Add source files to our target executable
add_executable( SKNativeTemplate
  src/main.cpp
)

# Link to dependencies
target_link_libraries( SKNativeTemplate
  StereoKitC
)