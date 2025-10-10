cmake_minimum_required(VERSION 3.24 FATAL_ERROR) # for PATH_EQUAL

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    add_compile_options(/W4)
    add_compile_options(/wd4505)
    add_compile_options(/WX)
endif()

set(EXE_NAME Parser)
project(${EXE_NAME} LANGUAGES C CXX)

add_subdirectory(assets)
add_subdirectory(dependencies)
add_subdirectory(LibMath)
add_subdirectory(Resources)
add_subdirectory(LowRenderer)
add_subdirectory(ModernOpenGL)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Parser)
endif()