cmake_minimum_required(VERSION 3.20)
project(OpenGL_Workshop)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor)

add_executable(OpenGL_Workshop main.cpp)

target_link_libraries(OpenGL_Workshop vendor)
