cmake_minimum_required(VERSION 3.16)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(live2d_texture_utils)

find_package(Boost)

add_library(nothing STATIC)
target_sources(nothing PRIVATE nothing_lib.c)
target_include_directories(nothing PUBLIC nothing)
target_compile_definitions(nothing PUBLIC -DSTBIR_SSE2)

add_executable(pngutil)
target_sources(pngutil PRIVATE
    pngutil.cpp
)
target_link_libraries(pngutil PRIVATE nothing)
if (${Boost_FOUND})
    target_include_directories(pngutil PRIVATE ${Boost_INCLUDE_DIR})
    target_compile_definitions(pngutil PRIVATE -DHAVE_BOOST=1)
endif()
