cmake_minimum_required(VERSION 3.16)
project(stellarcanvas_native LANGUAGES CXX)

add_library(stellarcanvas SHARED
    napi_init.cpp
)

target_compile_features(stellarcanvas PRIVATE cxx_std_17)
target_compile_options(stellarcanvas PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(stellarcanvas PRIVATE
    libace_napi.z.so
    libhilog_ndk.z.so
)
