#!/bin/bash

docker build --network=host -t stereokit/linux_x64 -f x64linux.Dockerfile . && docker run --network=host -v "/$(pwd)/../..":/var/projects/StereoKit stereokit/linux_x64

docker build --network=host -t stereokit/linux_arm64 -f arm64linux.Dockerfile . && docker run --network=host -v "/$(pwd)/../..":/var/projects/StereoKit stereokit/linux_arm64
