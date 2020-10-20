##
#  Author: Jennifer Faj
#  eMail: jfaj@mail.uni-paderborn.de
#  Date:  2020/06/10
#

set(KERNEL_SRC_DIR "ransac")
set(KERNEL_FV "ransac_fv")
set(KERNEL_FV_SRC "${PROJECT_SOURCE_DIR}/src/${KERNEL_SRC_DIR}/${KERNEL_FV}.cl")
set(KERNEL_FV_GLOBAL "ransac_fv_global")
set(KERNEL_FV_GLOBAL_SRC "${PROJECT_SOURCE_DIR}/src/${KERNEL_SRC_DIR}/${KERNEL_FV_GLOBAL}.cl")
set(KERNEL_P "ransac_p")
set(KERNEL_P_SRC "${PROJECT_SOURCE_DIR}/src/${KERNEL_SRC_DIR}/${KERNEL_P}.cl")
set(TARGET_BOARD "p520_hpc_sg280l")

#
# Add other dependencies here, if there are any (like specific libraries etc.?)
# Add Benchmark specific compilation options here, if there are any
#

# usage example: cmake ../ -DRANSAC_PIPELINE_WIDTH=0
set(RANSAC_PO "4" CACHE STRING "Choose number of parallel outlier computations")
set(RANSAC_CU "1" CACHE STRING "Choose number of compute units")
set(RANSAC_N "5888" CACHE STRING "Choose number of parallel outlier computations")
set(RANSAC_COMPILE_DEF_1 "-DPO=${RANSAC_PO}")
set(RANSAC_COMPILE_DEF_2 "-DCU=${RANSAC_CU}")
set(RANSAC_COMPILE_DEF_3 "-DN=${RANSAC_N}")
message("RANSAC: setting preprocessor directives ${RANSAC_COMPILE_DEF_1}, ${RANSAC_COMPILE_DEF_2} and ${RANSAC_COMPILE_DEF_3}")

add_library(ransacutility ransacutility.cpp)
target_include_directories(ransacutility PUBLIC ../ransac)

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
add_custom_target(${KERNEL_FV}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_FV_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV}_emulate
                  DEPENDS ${KERNEL_FV_SRC})
add_custom_target(${KERNEL_FV_GLOBAL}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_FV_GLOBAL_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV_GLOBAL}_emulate
                  DEPENDS ${KERNEL_FV_GLOBAL_SRC})
add_custom_target(${KERNEL_P}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_P_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_P}_emulate
                  DEPENDS ${KERNEL_P_SRC})

# report generation
add_custom_target(${KERNEL_FV}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_FV_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV}_report
                  DEPENDS ${KERNEL_FV_SRC})
add_custom_target(${KERNEL_FV_GLOBAL}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_FV_GLOBAL_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV_GLOBAL}_report
                  DEPENDS ${KERNEL_FV_GLOBAL_SRC})
add_custom_target(${KERNEL_P}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_P_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_P}_report
                  DEPENDS ${KERNEL_P_SRC})

# synthesis
add_custom_target(${KERNEL_FV}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_FV_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV}_synthesis
                  DEPENDS ${KERNEL_FV_SRC})
add_custom_target(${KERNEL_FV_GLOBAL}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} -no-interleaving=DDR ${KERNEL_FV_GLOBAL_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV_GLOBAL}_synthesis
                  DEPENDS ${KERNEL_FV_GLOBAL_SRC})
add_custom_target(${KERNEL_P}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_P_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_P}_synthesis
                  DEPENDS ${KERNEL_P_SRC})

# synthesis + profiling
add_custom_target(${KERNEL_FV}_profile
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} -profile ${KERNEL_FV_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV}_profile
                  DEPENDS ${KERNEL_FV_SRC})
add_custom_target(${KERNEL_FV_GLOBAL}_profile
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} -profile -no-interleaving=DDR ${KERNEL_FV_GLOBAL_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_FV_GLOBAL}_profile
                  DEPENDS ${KERNEL_FV_GLOBAL_SRC})
add_custom_target(${KERNEL_P}_profile
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} -profile ${KERNEL_P_SRC} ${RANSAC_COMPILE_DEF_1} ${RANSAC_COMPILE_DEF_2} ${RANSAC_COMPILE_DEF_3} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_P}_profile
                  DEPENDS ${KERNEL_P_SRC})
