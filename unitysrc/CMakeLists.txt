cmake_minimum_required (VERSION 3.12)

set(SFML_STATIC_LIBRARIES TRUE)
#set(SFML_DIR "${PROJECT_SOURCE_DIR}/ext/SFML/lib/cmake/SFML") #maybe

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)
find_package(TBB REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.68.0 COMPONENTS filesystem thread serialization REQUIRED)

add_executable(HourglassII WIN32 MACOSX_BUNDLE)

#TODO: Separate into Debug and Release, and per-platform config?
target_compile_definitions(HourglassII
    PRIVATE
        BOOST_RANGE_ENABLE_CONCEPT_ASSERT=0
        GLEW_BUILD
        BOOST_THREAD_VERSION=4
        #Windows or MSVC specific:
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        NOMINMAX
        WIN32
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
        _WINDOWS
        _UNICODE
        UNICODE
)

set_target_properties(HourglassII PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

target_sources(HourglassII
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/FrontEnd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/glew.cpp
        ${CMAKE_CURRENT_LIST_DIR}/lua.cpp
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PhysicsEngine.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TimeEngine.cpp
        #Windows Specific
        ${CMAKE_CURRENT_LIST_DIR}/../src/windows/resource.rc
)

target_link_libraries(HourglassII sfml-graphics sfml-audio)
target_link_libraries(HourglassII Boost::boost Boost::filesystem Boost::thread Boost::serialization)
target_link_libraries(HourglassII TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy)

#Not copying data files or dlls.
#This should be done manually for now.
#(Perhaps with a symbolic link to the data folder, so that data files can be edited while they are being used).
