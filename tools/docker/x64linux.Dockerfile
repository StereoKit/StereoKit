FROM ubuntu:22.04

COPY runbuild-x64.sh /var/

RUN apt-get update
RUN apt-get install -y build-essential cmake ninja-build clang git
RUN apt-get install -y lld libfontconfig1-dev libgl1-mesa-dev libx11-xcb-dev libxfixes-dev libxcb-dri2-0-dev libxcb-glx0-dev libxcb-icccm4-dev libxcb-keysyms1-dev libxcb-randr0-dev libxrandr-dev libxxf86vm-dev mesa-common-dev

WORKDIR /var/projects/StereoKit
ENTRYPOINT ["/var/runbuild-x64.sh"]
