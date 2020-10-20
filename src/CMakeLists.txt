##
#  Author: Jennifer Faj
#  eMail: jfaj@mail.uni-paderborn.de
#  Date:  2020/02/18
#

# AOCL INIT

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ../cmake)
find_package(IntelFPGAOpenCL)

# Dependencies
include_directories(${IntelFPGAOpenCL_INCLUDE_DIRS})

# HOST INIT

# Host compiler & linker flags
set(CMAKE_CXX_FLAGS "-lgmp -DQUARTUS_MAJOR_VERSION=${IntelFPGAOpenCL_MAJOR_VERSION}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")


# Avoid conflicts with previous version
set(AOC ${IntelFPGAOpenCL_AOC})

# Add common host libraries
add_subdirectory(common)

# ------- Benchmarks --------- #
add_subdirectory(md5)
add_subdirectory(scan)
add_subdirectory(firfilter)
add_subdirectory(mm)
add_subdirectory(nw)
add_subdirectory(ransac)
add_subdirectory(mergesort)

add_definitions(-DBSIZE=${NWBSIZE} -DPAR=${NWPAR})

add_executable(mainhost mainhost.cpp)

# ------- Benchmarks --------- #
target_sources(mainhost PRIVATE
               md5/md5host.cpp
               scan/scanhost.cpp
               firfilter/firfilterhost.cpp
               nw/nwhost.cpp
               mm/mmhost.cpp
               ransac/ransachost.cpp
               mergesort/mergesorthost.cpp)


target_link_libraries(mainhost PUBLIC
                      ${IntelFPGAOpenCL_LIBRARIES}
                      utility
                      benchmarkoptionsparser
                      benchmarkdatabase
                      timer
                      firfilterutility
                      ransacutility)

# add binary directory
set_target_properties(mainhost
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# compile all kernels for emulation
add_custom_target(all_kernels_emulate
                  DEPENDS scan_emulate
                  DEPENDS md5_emulate
                  DEPENDS nw_emulate
                  DEPENDS firfilter_single_man_mac_emulate
                  DEPENDS ransac_fv_emulate
                  DEPENDS ransac_fv_global_emulate
                  DEPENDS ransac_p_emulate)

# generate reports of all kernels
add_custom_target(all_kernels_report
                  DEPENDS scan_report
                  DEPENDS md5_report
                  DEPENDS nw_report
                  DEPENDS firfilter_single_man_mac_report
                  DEPENDS ransac_fv_report
                  DEPENDS ransac_fv_global_report
                  DEPENDS ransac_p_report)

# synthesize all kernels
add_custom_target(all_kernels_synthesis
                  DEPENDS scan_synthesis
                  DEPENDS md5_synthesis
                  DEPENDS nw_synthesis
                  DEPENDS firfilter_single_man_mac_synthesis
                  DEPENDS ransac_fv_synthesis
                  DEPENDS ransac_fv_global_synthesis
                  DEPENDS ransac_p_synthesis)
