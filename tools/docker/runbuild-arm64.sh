#!/bin/bash

cd /var/projects/StereoKit/
cmake --preset Linux_Arm64_Release
cmake --build --preset Linux_Arm64_Release
