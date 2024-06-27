FROM mcr.microsoft.com/devcontainers/base:jammy

# C++ development environment, uses clang, libc++, and ninja
ENV CC=clang
ENV CXX=clang++
ENV LDFLAGS="-stdlib=libc++ -fuse-ld=lld"
ENV CXXFLAGS=-stdlib=libc++
ENV CMAKE_GENERATOR=Ninja

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    # SDL2
    libsdl2-dev \
    libsdl2-image-dev \
    # C++ tools
    clang \
    clang-format \
    clang-tidy \
    clangd \
    libc++-dev \
    libc++abi-dev \
    lldb \
    lld \
    ninja-build \
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
    # Wayland
    # libwayland-dev \
    # libwayland-egl-backend-dev \
    # wayland-protocols \
    # OpenGL
    mesa-utils \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# set XDG_RUNTIME_DIR environment variable
ENV XDG_RUNTIME_DIR=/tmp
