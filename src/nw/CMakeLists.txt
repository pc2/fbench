##
#  Author: Masood Raeisi
#     Based on work from: Jennifer Faj <jfaj@mail.uni-paderborn.de>
#  eMail: mraeisi@mail.upb.de
#  Date:  2020/06/15
#

# REMARK: for now kernel version needs to be changed here
set(KERNEL "nw") 
set(KERNEL_SRC "${PROJECT_SOURCE_DIR}/src/${KERNEL}/${KERNEL}.cl")
set(TARGET_BOARD "p520_hpc_sg280l")

#
# Add other dependencies here, if there are any (like specific libraries etc.?)
# Add Benchmark specific compilation options here, if there are any
#
set(NWBSIZE 16 CACHE STRING "Choose Block Size")
set(NWPAR 4 CACHE STRING "Choose parallelism level")
set(NW_DEF_1 "-DBSIZE=${NWBSIZE}")
set(NW_DEF_2 "-DPAR=${NWPAR}")
message("NW: setting preprocessor directives ${NW_DEF_1} and ${NW_DEF_2}")

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
    -no-interleaving=default
    -board=${TARGET_BOARD})

# compile for emulation
add_custom_target(${KERNEL}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC} ${NW_DEF_1} ${NW_DEF_2} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_emulate
                  DEPENDS ${KERNEL_SRC})  

# report generation
add_custom_target(${KERNEL}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC} ${NW_DEF_1} ${NW_DEF_2} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_report 
                  DEPENDS ${KERNEL_SRC})
# synthesis
add_custom_target(${KERNEL}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC} ${NW_DEF_1} ${NW_DEF_2} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_synthesis
                  DEPENDS ${KERNEL_SRC})

