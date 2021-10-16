cmake_minimum_required(VERSION 3.13)

project(simple_library_example
        VERSION 0.1
        DESCRIPTION "An example application that simulates a typical image parsing program")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(img_lib STATIC lib.c)
target_include_directories(img_lib PUBLIC ${CMAKE_SOURCE_DIR})

file(GLOB WASM_RT ${CMAKE_SOURCE_DIR}/wasmrt/*.c)

add_library(img_lib_wasm STATIC ${WASM_RT} "${CMAKE_SOURCE_DIR}/wasm_prebuilt/lib_wasm.c")
target_include_directories(img_lib_wasm PUBLIC "${CMAKE_SOURCE_DIR}/wasm_prebuilt"
                                               "${CMAKE_SOURCE_DIR}/wasmrt/")

############################

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-comment)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
  add_compile_options(-fsanitize=undefined)
  add_link_options(-fsanitize=undefined)
endif()

add_executable(img_app main.cpp)
target_include_directories(img_app PUBLIC "${CMAKE_SOURCE_DIR}/rlbox/"
                                          "${CMAKE_SOURCE_DIR}/rlbox/"
                                          "${CMAKE_SOURCE_DIR}/wasm_prebuilt")
target_link_libraries(img_app img_lib img_lib_wasm)

add_executable(img_app_solution solution.cpp)
target_include_directories(img_app_solution PUBLIC "${CMAKE_SOURCE_DIR}/rlbox/"
                                                   "${CMAKE_SOURCE_DIR}/rlbox/"
                                                   "${CMAKE_SOURCE_DIR}/wasm_prebuilt")
target_link_libraries(img_app_solution img_lib img_lib_wasm)

add_custom_target(run COMMAND $<TARGET_FILE:img_app>)
add_dependencies(run img_app)

add_custom_target(run_solution COMMAND $<TARGET_FILE:img_app_solution>)
add_dependencies(run_solution img_app_solution)
