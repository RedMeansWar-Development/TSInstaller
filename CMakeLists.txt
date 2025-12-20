cmake_minimum_required(VERSION 3.16)

project(TSInstaller
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TSInstaller
    src/main.cpp
    third_party/miniz.c
)

target_include_directories(TSInstaller
    PRIVATE
        third_party/miniz
)
