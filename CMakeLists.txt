cmake_minimum_required(VERSION 3.23)
project(normalize)

set(CMAKE_CXX_STANDARD 20)

add_executable(normalize main.cpp)
add_executable(pro-12 pro-12.cpp)

target_link_libraries(pro-12 FreeImage)