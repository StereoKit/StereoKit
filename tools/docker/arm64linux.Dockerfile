FROM ubuntu:22.04

COPY runbuild-arm64.sh /var/
COPY arm-cross-compile-sources.list /etc/apt/sources.list.d/
RUN sed -i "s/deb http/deb [arch=amd64] http/g" /etc/apt/sources.list
RUN dpkg --add-architecture arm64

RUN apt-get update
RUN apt-get install -y build-essential cmake ninja-build clang git
RUN apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
RUN apt-get install -y lld:arm64 libfontconfig1-dev:arm64 libgl1-mesa-dev:arm64 libx11-xcb-dev:arm64 libxfixes-dev:arm64 libxcb-dri2-0-dev:arm64 libxcb-glx0-dev:arm64 libxcb-icccm4-dev:arm64 libxcb-keysyms1-dev:arm64 libxcb-randr0-dev:arm64 libxrandr-dev:arm64 libxxf86vm-dev:arm64 mesa-common-dev:arm64
WORKDIR /var/projects/StereoKit
ENTRYPOINT ["/var/runbuild-arm64.sh"]
