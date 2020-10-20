##
#  Author: Abdul Rehman Tareen
#  eMail: arktt@mail.uni-paderborn.de
#  Date:  2020/05/14
#

set(KERNEL "firfilter") 

# String keywords used 
set(SINGLE "single")
set(DOUBLE "double")
set(CHANNEL "channel")
set(MAN_MAC "man_mac")
set(UNRLL_MAC "unroll_mac")
set(NON_ALIGN "non_align")

# Kernel names

set(KERNEL_SING "${KERNEL}_${SINGLE}")
set(KERNEL_SING_UNRLL "${KERNEL}_${SINGLE}_${UNRLL_MAC}")
set(KERNEL_SING_MAN "${KERNEL}_${SINGLE}_${MAN_MAC}")
set(KERNEL_SING_MAN_NON_ALIGN "${KERNEL}_${SINGLE}_${MAN_MAC}_${NON_ALIGN}")
set(KERNEL_SING_CHAN_MAN_MAC_NON_ALIGN "${KERNEL}_${SINGLE}_${CHANNEL}_${MAN_MAC}_${NON_ALIGN}")

set(KERNEL_DOUBLE "${KERNEL}_${DOUBLE}")
set(KERNEL_DOUBLE_UNRLL "${KERNEL}_${DOUBLE}_${UNRLL_MAC}")

# Kernel files
set(KERNEL_SRC_SING "${PROJECT_SOURCE_DIR}/src/firfilter/${KERNEL}_${SINGLE}.cl")
set(KERNEL_SRC_SING_UNRLL "${PROJECT_SOURCE_DIR}/src/firfilter/${KERNEL}_${SINGLE}_${UNRLL_MAC}.cl")
set(KERNEL_SRC_SING_MAN "${PROJECT_SOURCE_DIR}/src/firfilter/${KERNEL}_${SINGLE}_${MAN_MAC}.cl")
set(KERNEL_SRC_SING_MAN_NON_ALIGN "${PROJECT_SOURCE_DIR}/src/firfilter/${KERNEL}_${SINGLE}_${MAN_MAC}_${NON_ALIGN}.cl")
set(KERNEL_SRC_SING_CHAN_MAN_MAC_NON_ALIGN "${PROJECT_SOURCE_DIR}/src/firfilter/${KERNEL}_${SINGLE}_${CHANNEL}_${MAN_MAC}_${NON_ALIGN}.cl")

set(KERNEL_SRC_DOUBLE "${PROJECT_SOURCE_DIR}/src/firfilter/${KERNEL}_${DOUBLE}.cl")
set(KERNEL_SRC_DOUBLE_UNRLL "${PROJECT_SOURCE_DIR}/src/firfilter/${KERNEL}_${DOUBLE}_${UNRLL_MAC}.cl")

# Synth target board
set(TARGET_BOARD "p520_hpc_sg280l")

#
# Add other dependencies here, if there are any (like specific libraries etc.?)
# Add Benchmark specific compilation options here, if there are any
#

set(DEF_TAP_SIZE "-DTAP_SIZE=256")
set(DEF_BLOCK_SIZE "-DBLOCK_SIZE=16")
set(DEF_MEM_BLOCK_SIZE "-DMEM_BLOCK_SIZE=16")
set(DEF_BLOCK_MEM_LCM "-DBLOCK_MEM_LCM=16")

set(COMPILE_DEF ${DEF_TAP_SIZE} ${DEF_TAP_SIZE} ${DEF_BLOCK_SIZE} ${DEF_MEM_BLOCK_SIZE} ${DEF_BLOCK_MEM_LCM})

message("preprocessor directive ${COMPILE_DEF}")

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

# Add Utilities
add_library(firfilterutility firfilterutility.cpp)
target_include_directories(firfilterutility PUBLIC ../firfilter)


# compile for emulation

add_custom_target(${KERNEL_SING}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC_SING} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING}_emulate.aocx
                  DEPENDS ${KERNEL_SRC_SING})

add_custom_target(${KERNEL_SING_UNRLL}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC_SING_UNRLL} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_UNRLL}_emulate.aocx
                  DEPENDS ${KERNEL_SRC_SING_UNRLL})

add_custom_target(${KERNEL_SING_MAN}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC_SING_MAN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_MAN}_emulate.aocx
                  DEPENDS ${KERNEL_SRC_SING_MAN})

add_custom_target(${KERNEL_SING_MAN_NON_ALIGN}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC_SING_MAN_NON_ALIGN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_MAN_NON_ALIGN}_emulate.aocx
                  DEPENDS ${KERNEL_SRC_SING_MAN_NON_ALIGN})

add_custom_target(${KERNEL_SING_CHAN_MAN_MAC_NON_ALIGN}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC_SING_CHAN_MAN_MAC_NON_ALIGN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_CHAN_MAN_MAC_NON_ALIGN}_emulate.aocx
                  DEPENDS ${KERNEL_SRC_SING_CHAN_MAN_MAC_NON_ALIGN})

add_custom_target(${KERNEL_DOUBLE}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC_DOUBLE} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_DOUBLE}_emulate.aocx
                  DEPENDS ${KERNEL_SRC_DOUBLE})             

add_custom_target(${KERNEL_DOUBLE_UNRLL}_emulate
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC_DOUBLE_UNRLL} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_DOUBLE_UNRLL}_emulate.aocx
                  DEPENDS ${KERNEL_SRC_DOUBLE_UNRLL}) 

# report generation

add_custom_target(${KERNEL_SING}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC_SING} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING}_report
                  DEPENDS ${KERNEL_SRC_SING})

add_custom_target(${KERNEL_SING_UNRLL}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC_SING_UNRLL} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_UNRLL}_report
                  DEPENDS ${KERNEL_SRC_SING_UNRLL})

add_custom_target(${KERNEL_SING_MAN}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC_SING_MAN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_MAN}_report
                  DEPENDS ${KERNEL_SRC_SING_MAN})

add_custom_target(${KERNEL_SING_MAN_NON_ALIGN}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC_SING_MAN_NON_ALIGN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_MAN_NON_ALIGN}_report
                  DEPENDS ${KERNEL_SRC_SING_MAN_NON_ALIGN})

add_custom_target(${KERNEL_SING_CHAN_MAN_MAC_NON_ALIGN}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC_SING_CHAN_MAN_MAC_NON_ALIGN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_CHAN_MAN_MAC_NON_ALIGN}_report
                  DEPENDS ${KERNEL_SRC_SING_CHAN_MAN_MAC_NON_ALIGN})

add_custom_target(${KERNEL_DOUBLE}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC_DOUBLE} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_DOUBLE}_report
                  DEPENDS ${KERNEL_SRC_DOUBLE})

add_custom_target(${KERNEL_DOUBLE_UNRLL}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC_DOUBLE_UNRLL} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_DOUBLE_UNRLL}_report
                  DEPENDS ${KERNEL_SRC_DOUBLE_UNRLL})

# synthesis

add_custom_target(${KERNEL_SING}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC_SING} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING}
                  DEPENDS ${KERNEL_SRC_SING})

add_custom_target(${KERNEL_SING_UNRLL}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC_SING_UNRLL} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_UNRLL}
                  DEPENDS ${KERNEL_SRC_SING_UNRLL})

add_custom_target(${KERNEL_SING_MAN}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC_SING_MAN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_MAN}
                  DEPENDS ${KERNEL_SRC_SING_MAN})

add_custom_target(${KERNEL_SING_MAN_NON_ALIGN}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC_SING_MAN_NON_ALIGN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_MAN_NON_ALIGN}
                  DEPENDS ${KERNEL_SRC_SING_MAN_NON_ALIGN})

add_custom_target(${KERNEL_SING_CHAN_MAN_MAC_NON_ALIGN}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC_SING_CHAN_MAN_MAC_NON_ALIGN} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_SING_CHAN_MAN_MAC_NON_ALIGN}
                  DEPENDS ${KERNEL_SRC_SING_CHAN_MAN_MAC_NON_ALIGN})

add_custom_target(${KERNEL_DOUBLE}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC_DOUBLE} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_DOUBLE}
                  DEPENDS ${KERNEL_SRC_DOUBLE})

add_custom_target(${KERNEL_DOUBLE_UNRLL}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC_DOUBLE_UNRLL} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL_DOUBLE_UNRLL}
                  DEPENDS ${KERNEL_SRC_DOUBLE_UNRLL})
