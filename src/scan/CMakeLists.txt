##
#  Author: Jennifer Faj
#  eMail: jfaj@mail.uni-paderborn.de
#  Date:  2020/02/19
#

# REMARK: for now kernel version needs to be changed here
set(KERNEL_VERSION "scan_v2")
set(KERNEL "scan") 
set(KERNEL_SRC "${PROJECT_SOURCE_DIR}/src/${KERNEL}/${KERNEL_VERSION}.cl")
set(TARGET_BOARD "p520_hpc_sg280l")

#
# Add other dependencies here, if there are any (like specific libraries etc.?)
# Add Benchmark specific compilation options here, if there are any
#
set(SCANBSIZE 16 CACHE STRING "Choose Block Size")
set(SCAN_DEF_1 "-DBLOCKSIZE=${SCANBSIZE}")
message("SCAN: setting preprocessor directives ${SCAN_DEF_1}")

# usage example: cmake ../ -DSCAN_LEVEL_OF_PARALLELISM=0
#set(SCAN_LEVEL_OF_PARALLELISM "2" CACHE STRING "Choose level of parallelism between 0 and 3" )
#set(COMPILE_DEF "-DNUM_VECTORS=4")
#
#if(SCAN_LEVEL_OF_PARALLELISM EQUAL "0")
#    set(COMPILE_DEF "-DNUM_VECTORS=1")
#elseif(SCAN_LEVEL_OF_PARALLELISM EQUAL "1")
#    set(COMPILE_DEF "-DNUM_VECTORS=2")
#elseif(SCAN_LEVEL_OF_PARALLELISM EQUAL "2")
#    set(COMPILE_DEF "-DNUM_VECTORS=4")
#elseif(SCAN_LEVEL_OF_PARALLELISM EQUAL "3")
#    set(COMPILE_DEF "-DNUM_VECTORS=8")
#else()
#    message("SCAN_LEVEL_OF_PARALLELISM: out of range")
#endif()
#message("SCAN: setting preprocessor directive ${COMPILE_DEF}")

# AOC compilation
set(AOC_EMULATION_PARAMS
    -march=emulator
    -legacy-emulator)
set(AOC_REPORT_PARAMS
    -rtl
    -report)
set(AOC_SYNTH_PARAMS
    -fpc
    -fp-relaxed
    -global-ring
    -duplicate-ring
    -board=${TARGET_BOARD})

# compile for emulation
add_custom_target(${KERNEL}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC} ${SCAN_DEF_1} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_emulate
                  DEPENDS ${KERNEL_SRC})  

# report generation
add_custom_target(${KERNEL}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC} ${SCAN_DEF_1} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_report 
                  DEPENDS ${KERNEL_SRC})
# synthesis
add_custom_target(${KERNEL}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC} ${SCAN_DEF_1} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_synthesis
                  DEPENDS ${KERNEL_SRC})

