##
#  Author: Jennifer Faj
#  eMail: jfaj@mail.uni-paderborn.de
#  Date:  2020/02/19
#

cmake_minimum_required(VERSION 3.15)

# Project settings
project(FBench 
        VERSION 1.0
        DESCRIPTION "An OpenCL Benchmark Suite for FPGAs")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(src)
# add_subdirectory(doc)

# Enable Building the Documentation
option(BUILD_DOC "Build documentation" ON)

# Testing

## Replace OFF with ON below to enable testing builds or ON with OFF to disable testing builds
option(PACKAGE_TESTS "Build the tests" ON)

if(PACKAGE_TESTS)

        enable_testing()

        # Download and unpack googletest at configure time
        configure_file(CMakeLists.txt.in test/googletest-download/CMakeLists.txt)

        execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test/googletest-download )

        if(result)
                message(FATAL_ERROR "CMake step for googletest failed: ${result}")
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test/googletest-download )

        if(result)
                message(FATAL_ERROR "Build step for googletest failed: ${result}")
        endif()

        # Prevent overriding the parent project's compiler/linker
        # settings on Windows
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        # Add googletest directly to our build. This defines
        # the gtest and gtest_main targets.
        add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/test/googletest-src
                        ${CMAKE_CURRENT_BINARY_DIR}/test/googletest-build
                        EXCLUDE_FROM_ALL)

        # The gtest/gtest_main targets carry header search path
        # dependencies automatically when using CMake 2.8.11 or
        # later. Otherwise we have to add them here ourselves.
        if (CMAKE_VERSION VERSION_LESS 2.8.11)
                include_directories("${gtest_SOURCE_DIR}/include")
        endif()

        add_subdirectory(test)

endif()

# Generation of API Documentation with Doxygen
find_package(Doxygen)
if(Doxygen_FOUND)
  configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile 
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	  COMMENT "Generating API documentation with Doxygen"
	  VERBATIM)
endif(Doxygen_FOUND)
