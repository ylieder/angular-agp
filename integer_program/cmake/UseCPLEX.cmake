# if CPLEX is found, this defines a target algutil::algcplex that can be linked against to setup everything for using cplex and our auxilliary header
cmake_minimum_required(VERSION 3.4)

if(NOT TARGET algutil::algcplex)
    if(COMMAND util_imported_link_libraries)
        find_package(CPLEX)

        if(CPLEX_FOUND)
            add_library(algutil::algcplex INTERFACE IMPORTED)
            set_target_properties(algutil::algcplex PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${_CPLEX_CMAKE_CURRENT_SOURCE_DIR}/include")
            util_imported_link_libraries(algutil::algcplex algutil::cplex)
        endif()
    else()
        message(ERROR "Before including UseCPLEX.cmake, please include util.cmake from the alg/cmake repository!")
    endif()
endif()