cmake_minimum_required(VERSION 3.1...3.14)
project(angular-art-gallery-problem)

set(CMAKE_CXX_STANDARD 17)

# Requires CGAL libraries
find_package(CGAL REQUIRED QUIET OPTIONAL_COMPONENTS Core)
include(${CGAL_USE_FILE})

# Requires Boost numeric (header only) and program options
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

include_directories(src)
include_directories(lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_LOG)
endif()

set(SOURCES
        src/upper_bound/pattern_manager.cpp
)

# AAGP solver tool
add_executable(aagp main.cpp ${SOURCES})
target_link_libraries(aagp LINK_PUBLIC ${Boost_LIBRARIES})

# AAGP benchmark tool
add_executable(benchmark benchmark.cpp ${SOURCES})
target_link_libraries(benchmark LINK_PUBLIC ${Boost_LIBRARIES})

# Random polygon generation
add_executable(generate_random generate_random.cpp ${SOURCES})
target_link_libraries(generate_random LINK_PUBLIC ${Boost_LIBRARIES})
