##
#  Author: Jennifer Faj
#  eMail: jfaj@mail.uni-paderborn.de
#  Date:  2020/02/19
#

set(KERNEL "md5") 
set(KERNEL_SRC "${PROJECT_SOURCE_DIR}/src/md5/${KERNEL}.cl")
set(TARGET_BOARD "p520_hpc_sg280l")

#
# Add other dependencies here, if there are any (like specific libraries etc.?)
# Add Benchmark specific compilation options here, if there are any
#

# usage example: cmake ../ -DMD5_LEVEL_OF_PARALLELISM=0
set(MD5_LEVEL_OF_PARALLELISM "16" CACHE STRING "Choose level of parallelism (divisor of keyspace sizes)" )
set(COMPILE_DEF "-DTERMINAL_LOOP_SIZE=16")

# keyspace sizes
# 10^7 -> 0x00989680
# 30^5 -> 0x0172C9E0 
# 25^6 -> 0x0E8D4A51
# 70^5 -> 0x642D7260

math(EXPR value_0 "0x00989680 % ${MD5_LEVEL_OF_PARALLELISM}" OUTPUT_FORMAT DECIMAL)
math(EXPR value_1 "0x0172C9E0 % ${MD5_LEVEL_OF_PARALLELISM}" OUTPUT_FORMAT DECIMAL)
math(EXPR value_2 "0x0E8D4A51 % ${MD5_LEVEL_OF_PARALLELISM}" OUTPUT_FORMAT DECIMAL)
math(EXPR value_3 "0x642D7260 % ${MD5_LEVEL_OF_PARALLELISM}" OUTPUT_FORMAT DECIMAL)

if((value_0 OR value_1 OR value_2 OR value_3) EQUAL 0)
    set(COMPILE_DEF "-DTERMINAL_LOOP_SIZE=${MD5_LEVEL_OF_PARALLELISM}")
else()
    message(WARNING "MD5_LEVEL_OF_PARALLELISM: input size not divisible by the provided number")
endif()
message("MD5: setting preprocessor directive ${COMPILE_DEF}")

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
