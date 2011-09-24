project(HourglassII CXX)
include(FindBoost)
include(FindFreetype)
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(BOOST_ROOT BOOST_ROOT-NOT_FOUND CACHE PATH "Path to boost installation")
set(TBB_INCLUDE_DIRS TBB_INCLUDE_DIRS-NOT_FOUND CACHE PATH "Path to threading building blocks include dir")

set(TBB_LIB_DIRS TBB_LIB_DIRS-NOT_FOUND CACHE PATH "path to threading building blocks libraries")

set(SFML_LIB_DIRS SFML_LIB_DIRS-NOT_FOUND CACHE PATH "path to Simple Fast Media Library libraries")

set(TBB_LIBS tbb tbbmalloc CACHE STRING "names of threading building blocks libraries")
set(SFML_LIBS sfml-system sfml-window sfml-graphics CACHE STRING "names of sfml libraries")

set(SFML_INCLUDE_DIRS SFML_INCLUDE_DIRS-NOT_FOUND CACHE PATH "include directory for SFML")

find_package(Boost 1.47.0 EXACT REQUIRED COMPONENTS filesystem serialization system)

find_package(Freetype)

include_directories(${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS} ${SFML_INCLUDE_DIRS})

file(GLOB sources *.cpp lua/*.cpp)

link_directories(
    ${TBB_LIB_DIRS}
    ${SFML_LIB_DIRS}
    ${Boost_LIBRARY_DIRS})

find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
find_library(COCOA_FRAMEWORK Cocoa)
find_library(OPENGL_FRAMEWORK OpenGL)

add_executable(HourglassII MACOSX_BUNDLE ${sources})

target_link_libraries(HourglassII ${CORE_FOUNDATION_FRAMEWORK} ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK})
target_link_libraries(HourglassII ${TBB_LIBS} ${SFML_LIBS} ${Boost_LIBRARIES} ${FREETYPE_LIBRARIES})

get_target_property(HourglassIIOutputName HourglassII OUTPUT_NAME)

add_custom_command(
    TARGET HourglassII POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
    ARGS -E copy_if_different
        ${TBB_LIB_DIRS}/libtbb.dylib
        ${HourglassIIOutputName}/Contents/Frameworks/libtbb.dylib)

add_custom_command(
    TARGET HourglassII POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_if_different
        ${TBB_LIB_DIRS}/libtbbmalloc.dylib
        ${HourglassIIOutputName}/Contents/Frameworks/libtbbmalloc.dylib)

add_custom_command(
    TARGET HourglassII POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_if_different
        triggerSystem.lua
        ${HourglassIIOutputName}/Contents/Resources/triggerSystem.lua)
