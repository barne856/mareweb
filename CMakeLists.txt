cmake_minimum_required(VERSION 3.28)
project(DawnProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
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

# SQUINT
FetchContent_Declare(
  SQUINT
  GIT_REPOSITORY https://github.com/barne856/squint.git
  GIT_TAG main
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
)

# Dawn has some specific build requirements
set(DAWN_FETCH_DEPENDENCIES ON)
set(DAWN_ENABLE_D3D11 OFF)
set(DAWN_ENABLE_D3D12 OFF)
set(DAWN_ENABLE_VULKAN ON)
set(DAWN_ENABLE_METAL OFF)
set(DAWN_ENABLE_NULL OFF)
set(DAWN_ENABLE_DESKTOP_GL OFF)
set(DAWN_ENABLE_OPENGLES OFF)
set(DAWN_USE_WAYLAND OFF)
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

# Configure SQUINT options
set(SQUINT_BUILD_TESTS OFF)

# Make squint available
FetchContent_MakeAvailable(SQUINT)

# Add include directories
include_directories(include)

# Find all .cpp files in the src directory
file(GLOB MAREWEB_SOURCES "src/*.cpp" "src/components/*.cpp" "src/entities/*.cpp" "src/systems/*.cpp")

# Create library target for mareweb
add_library(mareweb ${MAREWEB_SOURCES})

target_include_directories(mareweb PUBLIC include)
target_link_libraries(mareweb PUBLIC webgpu_cpp webgpu_dawn SDL3::SDL3 SDL3_image::SDL3_image SQUINT::SQUINT)

# Function to add an example executable
function(add_example NAME)
    add_executable(${NAME} examples/${NAME}.cpp)
    target_link_libraries(${NAME} PRIVATE mareweb)
    
    # Link gfortran only if SQUINT_BLAS_BACKEND is OpenBLAS
    if(UNIX)
        target_link_libraries(${NAME} PRIVATE gfortran)
    endif()
    
    # Copy only the necessary shared libraries to the output directory
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE:SDL3_image::SDL3_image>
            $<TARGET_FILE_DIR:${NAME}>
    )
endfunction()

# Find all .cpp files in the examples directory
file(GLOB EXAMPLE_SOURCES "examples/*.cpp")

# Add an executable for each example
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    add_example(${EXAMPLE_NAME})
endforeach()