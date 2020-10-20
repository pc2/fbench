##
#  Author: Akhtar, Junaid
#  eMail: junaida@mail.uni-paderborn.de
#  Date:  2020/03/26
#

# Find paths relative to the sdk root
set(AOCL_ROOT       "$ENV{INTELFPGAOCLSDKROOT}")
set(AOCL            "${AOCL_ROOT}/bin/aocl")
set(AOC             "${AOCL_ROOT}/bin/aoc")

# Get Quartus Version
set(QUARTUS_MAJOR_VERSION   "$ENV{QUARTUS_VERSION}")
string(REGEX MATCH "^([0-9]*)" QUARTUS_MAJOR_VERSION "${QUARTUS_MAJOR_VERSION}")
# Get aocl libraries to link against
execute_process(COMMAND bash -c "${AOCL} link-config" OUTPUT_VARIABLE AOCL_LINK_CONFIG)
string(REGEX MATCHALL "-L[^ \t\n]+" AOCL_LINKLIB_DIRS "${AOCL_LINK_CONFIG}")
string(REPLACE "-L" "" AOCL_LINKLIB_DIRS "${AOCL_LINKLIB_DIRS}")
# Get aocl include directories
execute_process(COMMAND bash -c "${AOCL} compile-config" OUTPUT_VARIABLE AOCL_COMPILE_CONFIG)
string(REGEX MATCHALL "-I[^ \t\n]+" AOCL_INCLUDE_DIRS "${AOCL_COMPILE_CONFIG}")
string(REPLACE "-I" "" AOCL_INCLUDE_DIRS "${AOCL_INCLUDE_DIRS}")

# Some additional (DEBUG) info
include(CMakePrintHelpers)
cmake_print_variables(QUARTUS_MAJOR_VERSION)
cmake_print_variables(AOCL_INCLUDE_DIRS)
cmake_print_variables(AOCL_LINKLIB_DIRS)

# Dependencies
include_directories(${AOCL_INCLUDE_DIRS})
link_directories(${AOCL_LINKLIB_DIRS})

# Host compiler & linker flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQUARTUS_MAJOR_VERSION=${QUARTUS_MAJOR_VERSION}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -lalteracl -lbitt_s10_pcie_mmd -lelf")


file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
set(SOURCES ${TEST_SOURCES})

add_definitions(-DBSIZE=${NWBSIZE} -DPAR=${NWPAR})

add_executable(maintest ${TEST_SOURCES})

target_link_libraries(maintest PUBLIC
                      utility
                      benchmarkoptionsparser
                      benchmarkdatabase
                      timer
                      firfilterutility
                      ransacutility)

target_link_libraries(maintest PUBLIC ${CMAKE_BINARY_DIR}/test/googletest-src gtest)

# add binary directory
set_target_properties(maintest
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/bin")

# pass arguments to test executable binary
# set(DeviceInitArgs  -p 0 -d 0 -v)
# add_test(maintest "${DeviceInitArgs}" AUTO)
