cmake_minimum_required (VERSION 3.12)

#set(SFML_DIR "${PROJECT_SOURCE_DIR}/ext/SFML/lib/cmake/SFML") #maybe

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.68.0 COMPONENTS filesystem thread serialization REQUIRED)

find_package(TBB REQUIRED)

find_package(glfw3 CONFIG REQUIRED)

find_package(Vulkan REQUIRED)

add_executable(HourglassII WIN32 MACOSX_BUNDLE)

#TODO: Separate into Debug and Release, and per-platform config?
target_compile_definitions(HourglassII
    PRIVATE
        GLEW_BUILD
        BOOST_THREAD_VERSION=4
        GLFW_INCLUDE_VULKAN
        #Windows and/or MSVC specific:
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        NOMINMAX
        WIN32
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
        _WINDOWS
        _UNICODE
        UNICODE
        VC_EXTRALEAN
        WIN32_LEAN_AND_MEAN
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

#Windows and/or MSVC specific:
target_compile_options(HourglassII
    PRIVATE
        /bigobj
        /fp:strict
        /permissive-
        /Zc:inline
        /Zc:throwingNew)

target_link_libraries(HourglassII sfml-graphics sfml-audio)
target_link_libraries(HourglassII Boost::boost Boost::filesystem Boost::thread Boost::serialization)
target_link_libraries(HourglassII TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy)
target_link_libraries(HourglassII glfw)
target_link_libraries(HourglassII Vulkan::Vulkan)


set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/shadersrc/*.glsl"
    )

message(${GLSL_SOURCE_FILES})

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/data/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/data/shaders/"
    #glslangValidator.exe only support SPIR-V output at an experimental level
    #at time of writing, and does not perform any optimization etc.
    #In the future it might be worth using a different compiler or even a different
    #shader source language.
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

file(GLOB_RECURSE DATA_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/data/*"
    )

STRING(LENGTH "${CMAKE_SOURCE_DIR}/" CMAKE_SOURCE_DIR_LEN)

foreach(DATA_SOURCE_FILE ${DATA_SOURCE_FILES})
    string(SUBSTRING ${DATA_SOURCE_FILE} ${CMAKE_SOURCE_DIR_LEN} -1 DATA_SOURCE_FILE_REL)
    list(APPEND DATA_SOURCE_FILES_REL ${DATA_SOURCE_FILE_REL})
endforeach()

foreach(DATA_SOURCE_FILE_REL ${DATA_SOURCE_FILES_REL})
    set(DATA_TARGET_FILE "${CMAKE_CURRENT_BINARY_DIR}/${DATA_SOURCE_FILE_REL}")
    list(APPEND DATA_TARGET_FILES ${DATA_TARGET_FILE})
endforeach()

add_custom_command(
    OUTPUT ${DATA_TARGET_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:HourglassII>/data"
    DEPENDS ${DATA_SOURCE_FILES}
)
#Still not copying DLLs, this needs to be done for a full hands-off build from source to be runnable
add_custom_target(HourglassIIPackage ALL VERBATIM DEPENDS ${DATA_TARGET_FILES} ${SPIRV_BINARY_FILES})
add_dependencies(HourglassIIPackage HourglassII)
