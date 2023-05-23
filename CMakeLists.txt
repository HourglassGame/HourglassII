cmake_minimum_required(VERSION 3.26...3.26 FATAL_ERROR)


project(
    "HourglassII"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

add_subdirectory("unitysrc")
