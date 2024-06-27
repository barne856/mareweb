cmake_minimum_required(VERSION 3.28)
project(DawnProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Dawn
FetchContent_Declare(
  dawn
  GIT_REPOSITORY https://dawn.googlesource.com/dawn
  GIT_TAG chromium/6554
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL # new in CMake 3.28
  GIT_SUBMODULES "docs" # workaround for https://gitlab.kitware.com/cmake/cmake/-/issues/20579
)

# SDL3
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG main
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  GIT_SUBMODULES "docs"
)

# SDL_image
FetchContent_Declare(
  SDL_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG main
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  GIT_SUBMODULES "docs"
)

# Dawn has some specific build requirements
set(DAWN_FETCH_DEPENDENCIES ON)
set(DAWN_ENABLE_D3D11 OFF)
set(DAWN_ENABLE_D3D12 OFF)
set(DAWN_ENABLE_METAL OFF)
set(DAWN_ENABLE_NULL OFF)
set(DAWN_ENABLE_DESKTOP_GL OFF)
set(DAWN_ENABLE_OPENGLES OFF)
set(DAWN_ENABLE_VULKAN ON)
set(DAWN_USE_WAYLAND OFF)
set(DAWN_USE_X11 ON)
set(TINT_BUILD_SPV_READER OFF)
# Disable unneeded parts
set(DAWN_BUILD_SAMPLES OFF)
set(TINT_BUILD_TINT OFF)
set(TINT_BUILD_SAMPLES OFF)
set(TINT_BUILD_DOCS OFF)
set(TINT_BUILD_TESTS OFF)
set(TINT_BUILD_FUZZERS OFF)
set(TINT_BUILD_SPIRV_TOOLS_FUZZER OFF)
set(TINT_BUILD_AST_FUZZER OFF)
set(TINT_BUILD_REGEX_FUZZER OFF)
set(TINT_BUILD_BENCHMARKS OFF)
set(TINT_BUILD_TESTS OFF)
set(TINT_BUILD_AS_OTHER_OS OFF)
set(TINT_BUILD_REMOTE_COMPILE OFF)

# Make Dawn available
FetchContent_MakeAvailable(dawn)

# Configure SDL3 options
set(SDL_SHARED ON)
set(SDL_STATIC OFF)

# Make SDL3 available
FetchContent_MakeAvailable(SDL3)

# Configure SDL_image options
set(SDL3IMAGE_SAMPLES OFF)
set(SDL3IMAGE_VENDORED OFF)
set(SDL3IMAGE_SHARED ON)
set(SDL3IMAGE_STATIC OFF)

# Make SDL_image available
FetchContent_MakeAvailable(SDL_image)

# Create executable
add_executable(DawnApp src/main.cpp)

include_directories(include)

# Link against Dawn and SDL libraries
target_link_libraries(DawnApp PRIVATE webgpu_cpp webgpu_dawn SDL3::SDL3 SDL3_image::SDL3_image)

# Copy only the necessary shared libraries to the output directory
add_custom_command(TARGET DawnApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE:SDL3_image::SDL3_image>
        $<TARGET_FILE_DIR:DawnApp>
)
