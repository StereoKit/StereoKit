# To use this template, 
cmake_minimum_required(VERSION 3.11)
project(SKNativeTemplate VERSION "0.1.0" LANGUAGES CXX C)

# Grab and build StereoKit from the GitHub repository. Here we're setting SK up
# as a statically linked library.
include(FetchContent)
set(SK_BUILD_TESTS       OFF)
set(SK_BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
  StereoKitC
  GIT_REPOSITORY https://github.com/StereoKit/StereoKit.git
  GIT_TAG        v0.3.5
)
FetchContent_MakeAvailable(StereoKitC)

# Add source files to our target executable
add_executable( SKNativeTemplate
  src/main.cpp
)

# Link to dependencies
target_link_libraries( SKNativeTemplate
  PRIVATE StereoKitC
)