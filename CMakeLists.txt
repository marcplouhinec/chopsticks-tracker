cmake_minimum_required(VERSION 3.10)

project(ChopsticksTracker)

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(OpenMP_C "${CMAKE_C_COMPILER}" CACHE STRING "" FORCE)
        set(OpenMP_C_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument" CACHE STRING "" FORCE)
        set(OpenMP_C_LIB_NAMES "libomp" "libgomp" "libiomp5" CACHE STRING "" FORCE)
        set(OpenMP_libomp_LIBRARY ${OpenMP_C_LIB_NAMES} CACHE STRING "" FORCE)
        set(OpenMP_libgomp_LIBRARY ${OpenMP_C_LIB_NAMES} CACHE STRING "" FORCE)
        set(OpenMP_libiomp5_LIBRARY ${OpenMP_C_LIB_NAMES} CACHE STRING "" FORCE)
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(OpenMP_CXX "${CMAKE_CXX_COMPILER}" CACHE STRING "" FORCE)
        set(OpenMP_CXX_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument" CACHE STRING "" FORCE)
        set(OpenMP_CXX_LIB_NAMES "libomp" "libgomp" "libiomp5" CACHE STRING "" FORCE)
        set(OpenMP_libomp_LIBRARY ${OpenMP_CXX_LIB_NAMES} CACHE STRING "" FORCE)
        set(OpenMP_libgomp_LIBRARY ${OpenMP_CXX_LIB_NAMES} CACHE STRING "" FORCE)
        set(OpenMP_libiomp5_LIBRARY ${OpenMP_CXX_LIB_NAMES} CACHE STRING "" FORCE)
    endif()
endif()

if(APPLE)
    set(Darknet_LIBS "libdark.dylib")
else()
    set(Darknet_LIBS "libdark.so")
endif()

if(APPLE)
    set(Boost_LIBS "boost_log.dylib" "boost_log_setup.dylib" "boost_program_options.dylib" "boost_filesystem.dylib")
else()
    set(Boost_LIBS "boost_log.so" "boost_log_setup.so" "boost_program_options.so" "boost_filesystem.so")
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

find_package(OpenCV REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories("/usr/local/include/")
include_directories("/usr/local/include/opencv4/")
include_directories("/usr/local/include/darknet/")
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories("/usr/local/lib/")

file(GLOB_RECURSE fileCollections "src/*.cpp")

add_executable(ChopsticksTracker ${fileCollections})
target_link_libraries(ChopsticksTracker ${OpenCV_LIBS})
target_link_libraries(ChopsticksTracker ${Darknet_LIBS})
target_link_libraries(ChopsticksTracker ${Boost_LIBS})