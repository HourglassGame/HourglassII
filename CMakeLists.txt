project(HourglassII CXX)
include(FindBoost)
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(BOOST_ROOT ~/Programming/boost_1_46_1/)

set(TBB_INCLUDE_DIRS ~/Programming/tbb30_174oss/include/)

set(TBB_LIBS tbb
        tbbmalloc)

set(SFML_LIBS sfml-system sfml-window sfml-graphics)

find_package(Boost 1.46.1 EXACT REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})

file (GLOB sources *.cpp)

link_directories(/home/evan/Programming/tbb30_174oss/lib/intel64/cc4.1.0_libc2.4_kernel2.6.16.21 
                 /home/evan/Programming/SFML-1.6/lib)

add_executable(HourglassII ${sources})

target_link_libraries(HourglassII ${TBB_LIBS} ${SFML_LIBS})

