##
#  Modifier: Akhtar, Junaid
#  Author: Jennifer Faj <jfaj@mail.uni-paderborn.de>
#  Mail: junaida@mail-uni-paderborn.de
#  Date: 2020/07/03
#

set(KERNEL "mm") 
set(KERNEL_SRC "${PROJECT_SOURCE_DIR}/src/mm/${KERNEL}.cl")
set(TARGET_BOARD "p520_hpc_sg280l")

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
                  COMMAND ${AOC} ${AOC_EMULATION_PARAMS} ${KERNEL_SRC} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_emulate
                  DEPENDS ${KERNEL_SRC})

# report generation
add_custom_target(${KERNEL}_report
                  COMMAND ${AOC} ${AOC_REPORT_PARAMS} ${KERNEL_SRC} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_report
                  DEPENDS ${KERNEL_SRC})
# synthesis
add_custom_target(${KERNEL}_synthesis
                  COMMAND ${AOC} ${AOC_SYNTH_PARAMS} ${KERNEL_SRC} ${COMPILE_DEF} -o ${CMAKE_BINARY_DIR}/bin/${KERNEL}_synthesis
                  DEPENDS ${KERNEL_SRC})
