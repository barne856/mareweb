# Use the base image
FROM mcr.microsoft.com/devcontainers/base:noble

# Argument for LLVM version
ARG LLVM_VERSION=17

# C++ development environment, uses clang, libc++, and ninja
ENV CC=clang-$LLVM_VERSION
ENV CXX=clang++-$LLVM_VERSION
ENV FC=gfortran
ENV CMAKE_GENERATOR=Ninja
ENV CXXFLAGS=-stdlib=libc++

# Add the LLVM repository and install the specified version of LLVM
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    wget \
    gnupg \
    software-properties-common \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && add-apt-repository "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-$LLVM_VERSION main" \
    && apt-get update \
    && apt-get -y install --no-install-recommends \
    clang-$LLVM_VERSION \
    clang-format-$LLVM_VERSION \
    clang-tidy-$LLVM_VERSION \
    clangd-$LLVM_VERSION \
    libc++-$LLVM_VERSION-dev \
    libc++abi-$LLVM_VERSION-dev \
    lldb-$LLVM_VERSION \
    lld-$LLVM_VERSION \
    ninja-build \
    gfortran \
    intel-mkl \
    # X11
    libx11-dev \
    libx11-xcb-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxinerama-dev \
    libxi-dev \
    # Vulkan
    libvulkan-dev \
    vulkan-tools \
    mesa-vulkan-drivers \
    && apt-get clean && rm -rf /var/lib/apt/lists/* \ 
    && ln -s /usr/bin/clang-$LLVM_VERSION /usr/bin/clang \
    && ln -s /usr/bin/clang++-$LLVM_VERSION /usr/bin/clang++ \
    && ln -s /usr/bin/clang-format-$LLVM_VERSION /usr/bin/clang-format \
    && ln -s /usr/bin/clang-tidy-$LLVM_VERSION /usr/bin/clang-tidy \
    && ln -s /usr/bin/clangd-$LLVM_VERSION /usr/bin/clangd \
    && ln -s /usr/bin/lldb-$LLVM_VERSION /usr/bin/lldb \
    && ln -s /usr/bin/lld-$LLVM_VERSION /usr/bin/lld \
    && ln -f -s /usr/bin/ld.lld-$LLVM_VERSION /usr/bin/ld 

# set XDG_RUNTIME_DIR environment variable
ENV XDG_RUNTIME_DIR=/tmp
