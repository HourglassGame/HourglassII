set(HOURGLASSII_EXT_DIR "${PROJECT_SOURCE_DIR}/ext")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(
    SFML 2.5.1
    REQUIRED
    COMPONENTS graphics audio
    CONFIG
    HINTS "${HOURGLASSII_EXT_DIR}"
)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(
    Boost 1.77.0
    REQUIRED
    COMPONENTS headers filesystem thread serialization
    CONFIG
    HINTS "${HOURGLASSII_EXT_DIR}"
)

find_package(
    TBB 2020.3.0 #Latest is now oneAPI TBB 2021.3.0; Not using this yet because oneAPI TBB (v2020.1...2020.3) appears to be incompatible with C++20 mode on MSVC 2019 (v16.11.2)
    REQUIRED
    CONFIG
    HINTS "${HOURGLASSII_EXT_DIR}"
)

find_package(
    glfw3 3.3.4
    REQUIRED
    CONFIG
    HINTS "${HOURGLASSII_EXT_DIR}"
)

find_package(
    Microsoft.GSL 3.1.0 #Lastest is 3.1.0; but there are much more recent changes in the main branch of the git repository
    REQUIRED
    CONFIG
    HINTS "${HOURGLASSII_EXT_DIR}"
)

find_package(Vulkan REQUIRED)

add_executable(HourglassII WIN32 MACOSX_BUNDLE)

set_target_properties(sfml-graphics sfml-audio sfml-system PROPERTIES
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
)

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

target_include_directories(HourglassII PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../src/
)

target_sources(HourglassII
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/extraTest_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/FrontEnd_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/glew_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/lua_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/LuaUtil_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/main_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PhysicsEngine_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TimeEngine_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Util_UnitySrc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VulkanUtil_UnitySrc.cpp
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

target_link_libraries(HourglassII PRIVATE sfml-graphics sfml-audio)
target_link_libraries(HourglassII PRIVATE Boost::headers Boost::filesystem Boost::thread Boost::serialization)
target_link_libraries(HourglassII PRIVATE TBB::tbb TBB::tbbmalloc)
target_link_libraries(HourglassII PRIVATE glfw)
target_link_libraries(HourglassII PRIVATE Vulkan::Vulkan)
target_link_libraries(HourglassII PRIVATE Microsoft.GSL::GSL)

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

#file(GLOB_RECURSE DATA_SOURCE_FILES
#    "${CMAKE_SOURCE_DIR}/data/*"
#)

#STRING(LENGTH "${CMAKE_SOURCE_DIR}/" CMAKE_SOURCE_DIR_LEN)

#foreach(DATA_SOURCE_FILE ${DATA_SOURCE_FILES})
#    string(SUBSTRING ${DATA_SOURCE_FILE} ${CMAKE_SOURCE_DIR_LEN} -1 DATA_SOURCE_FILE_REL)
#    list(APPEND DATA_SOURCE_FILES_REL ${DATA_SOURCE_FILE_REL})
#endforeach()

#foreach(DATA_SOURCE_FILE_REL ${DATA_SOURCE_FILES_REL})
#    set(DATA_TARGET_FILE "${CMAKE_CURRENT_BINARY_DIR}/${DATA_SOURCE_FILE_REL}")
#    list(APPEND DATA_TARGET_FILES ${DATA_TARGET_FILE})
#endforeach()

#add_custom_command(
#    OUTPUT ${DATA_TARGET_FILES}
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_SOURCE_DIR}/data"
#        "$<TARGET_FILE_DIR:HourglassII>/data"
#    DEPENDS ${DATA_SOURCE_FILES}
#)
#Still not copying DLLs, this needs to be done for a full hands-off build from source to be runnable

#Relevant DLLs are:
#openal.dll (use openal-soft; https://openal-soft.org/)
#tbb.dll
#tbbmalloc.dll
#add_custom_target(HourglassIIPackage ALL VERBATIM DEPENDS ${DATA_TARGET_FILES} ${SPIRV_BINARY_FILES})
#add_dependencies(HourglassIIPackage HourglassII)
