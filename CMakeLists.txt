cmake_minimum_required(VERSION 3.23)
project(normalize)

set(CMAKE_CXX_STANDARD 20)


find_package(OpenMP REQUIRED)
if (OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

add_executable(normalize main.cpp)
add_executable(pro-12 pro-12.cpp)
add_executable(pro-13 pro-13.cpp)
add_executable(test test.cpp)

target_link_libraries(pro-13 FreeImage)