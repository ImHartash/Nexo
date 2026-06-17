cmake_minimum_required(VERSION 3.24)

project(Nexo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG
    ${CMAKE_SOURCE_DIR}/bin/debug
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE
    ${CMAKE_SOURCE_DIR}/bin/release
)

include(FetchContent)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(tomlplusplus)

#
# Subprojects
#

add_subdirectory(NexoServer)
add_subdirectory(NexoClient)