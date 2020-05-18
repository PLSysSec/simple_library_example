cmake_minimum_required(VERSION 3.13)

project(simple_library_example
        VERSION 0.1
        DESCRIPTION "An example application that simulates a typical image parsing program")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(img_lib STATIC lib.c)
target_include_directories(img_lib PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(img_app main.cpp)
target_link_libraries(img_app img_lib)
