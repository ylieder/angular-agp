cmake_minimum_required(VERSION 3.13)
project(AngularArtGalleryProblem)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${PROJECT_SOURCE_DIR}/cmake
        )

set(UTIL_BOOST_LIBRARIES program_options filesystem)

find_package(CGAL REQUIRED)
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
include(${CGAL_USE_FILE})


include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake" NO_POLICY_SCOPE)

set(_CPLEX_CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/UseCPLEX.cmake")

add_subdirectory(upper_bound)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(AAGP main.cpp)
util_setup_target(AAGP LIBRARIES algutil::algcplex ${CMAKE_THREAD_LIBS_INIT})

add_executable(AAGP_Batch batch_processing.cpp)
util_setup_target(AAGP_Batch LIBRARIES algutil::algcplex ${CMAKE_THREAD_LIBS_INIT})

add_executable(Create_Random create_random.cpp)
util_setup_target(Create_Random LIBRARIES algutil::algcplex ${CMAKE_THREAD_LIBS_INIT})