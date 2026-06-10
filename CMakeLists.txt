cmake_minimum_required(VERSION 3.20)
project(Nexo CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_ROOT "C:/vs_include/boost/boost-1.90.0")
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

add_subdirectory(server)
add_subdirectory(client)